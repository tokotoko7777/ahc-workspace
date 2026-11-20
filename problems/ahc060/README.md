# AHC060 — Ice Cream Collection

[Official statement / scoring](https://atcoder.jp/contests/ahc060/tasks/ahc060_a) ·
[Official generator / visualizer](https://img.atcoder.jp/ahc060/wX0NuJxV.zip) ·
[Official results JSON](https://atcoder.jp/contests/ahc060/results/json).

Absolute score is the total number of unique flavor strings per shop.
The final contest ranks the sum across the same 150 official cases (larger
better); no system rerun. The official results metadata identifies
**Sweet_Sweet_Soul** as rank 1, but does not include their score. Both
direct case/score archive endpoints returned 403 and the possible ALE
rank-only archive returned 404. No proof of historical cases or numeric
winning score; strict comparison is **unjudged**.

## Delivery frontier

Both arms share exactly the same deterministic 360-action exploratory
prefix up to the next shop, flipping roughly half of visited trees while
favoring unexplored vertices. Thereafter tree flavors are fixed. Starting
with an empty cone at a shop, both propose paths of at most 20 moves and
count at most 1000 edge extensions per dispatch. Non-backtracking condition
persists across deliveries and flips.

The control samples non-backtracking walks. The new arm explores them by
length, stops extending on entering a shop, and deduplicates frontier
states by exact packed word, current and previous vertex. These three
values fully determine the set of possible future completions and resulting
inventories while the flavors are fixed. A width cap of 56 balances coverage;
lower bound distance-to-shop and seeded priorities settle ties. Both prefer
short novel deliveries and a legal fallback route when none are found.

| Fresh cases | Sample ↑ | Frontier ↑ | Win / tie / loss |
|---|---:|---:|---:|
| Dev (24) | 851.750 | 867.958 | 21 / 0 / 3 |
| Holdout (24) | 882.750 | 894.875 | 19 / 0 / 5 |

Holdout median gain 14. Held-out frontier mean edge checks 451,105 vs
969,708 for samples, despite the same cap per dispatch, and median solver
wall time 0.0305 s vs 0.0407 s. Max observed 0.087508 s across all
96 runs. All 96 official scores, move counts, permanent flavor changes
and no-backtracking conditions match an independent exact replay. Keep
frontier as branch default. [Logs](benchmark-results.csv).

The fallback cannot use `graph_bfs.hpp` without explicitly adding the last
vertex to state. A naive queue copied whole path vectors, so
[`shortest_nonbacktracking_path`](../../library/nonbacktracking-path.hpp)
uses parent reconstruction in directed-edge state space instead.
Its differential test compares with exact layer-by-layer state reachability.

**Next hypothesis:** let the frontier evaluate at most one legal flavor flip
along a sampled delivery, not just a frozen coloring. A flip should count
one extra action and change all following emissions from that tree without
changing its first harvest. Use an extra flag/vertex in the dedup key;
compare against a higher-budget frozen-color frontier with the same
untouched-case protocol. Avoid allocating an irreversible flip to a single
novel string if it destroys many cheap still-unseen future strings.

```sh
p=problems/ahc060
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC060_FRONTIER=0 "$p/main.cpp" -o "$d/base"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/frontier"
python "$p/benchmark.py" /path/to/gen /path/to/vis "$d/base" "$d/frontier" "$d/results.csv"
g++ -std=c++23 -O3 tests/nonbacktracking_path_test.cpp -o "$d/path-test"
"$d/path-test"
```
