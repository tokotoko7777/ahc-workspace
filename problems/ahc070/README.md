# AHC070 — Ward the Village

[Official rules](https://atcoder.jp/contests/ahc070/tasks/ahc070_a) ·
[Official generator and visualizer](https://img.atcoder.jp/ahc070/f4sYgUzY.zip) ·
[Official results](https://atcoder.jp/contests/ahc070/results/json).

The score is `round(10^6*N^3/(D+1))`, where
`D = sum(floor(nearest_distance[t]*sqrt(t+1)))`. Movement wraps around
the grid, but nearest-source Manhattan distance does not. The final
ranking sums the 150 original contest case scores, larger better, with
no system rerun. Official ranked metadata names **eijirou** first,
without score. Both direct case/score CSVs returned 403 and the possible
ALE rank-only fallback 404. Numeric target and identical historical cases
are unverified, so strict comparison is **unjudged**.

## Near translations versus long spreads

The first two translations are fixed axis unit steps. Both arms preserve
the same fallback and compare eight extra paths with identical budgets
and scoring. The control varies a third short translation; the candidate
varies long non-collinear displacements that spread new wards beyond
the immediate vicinity. Every trial has exactly 10,000 moves. At each turn,
rank three neighbors by current and next 14 attacks' estimated marginal
distance improvements plus nearest-disconnected coverage value, then
add a discounted two-step forecast from the best next translation.
All completed routes are compared on exact raw danger.

| Fresh cases | Near danger ↓ | Spread danger ↓ | Near score ↑ | Spread score ↑ |
|---|---:|---:|---:|---:|
| Dev (24) | 393,963.25 | 351,279.96 | 2,539,062.42 | 2,846,872.46 |
| Holdout (24) | 393,179.42 | 352,032.04 | 2,544,383.00 | 2,840,819.13 |

Long translations win all 48, holdout median danger reduction 40,717,
and have lower median wall time (.02294 vs .03383 s). All 96 runs match
official scoring, independent placement-time and distance-ring replay,
and exactly eight additional trials. Max wall .062739 s; spread is default.
[Paired logs](benchmark-results.csv).

A static 2D distance transform cannot be rerun every turn cheaply here.
The new
[`IncrementalManhattanDistance`](../../library/incremental-manhattan-distance.hpp)
propagates only strict improvements after adding one source, and reuses
queue storage. The differential test checks 6,000 incremental operations
against full minimum over inserted sources, including duplicates.

**Next hypothesis:** current two-step forecasts can give both placements
credit for fixing the same future event. At equal pair probes compute exact
new nearest distances with the first and second placed at their true times,
and choose the first move after testing both attacks rather than adding two
overlapping speculative marginal scores. Compare unseen score and ending
coverage, and keep the common old fallback.

```sh
p=problems/ahc070
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC070_SPREAD=0 "$p/main.cpp" -o "$d/near"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/spread"
python "$p/benchmark.py" /path/to/gen /path/to/vis "$d/near" "$d/spread" "$d/results.csv"
g++ -std=c++23 -O3 tests/incremental_manhattan_distance_test.cpp -o "$d/test"
"$d/test"
```
