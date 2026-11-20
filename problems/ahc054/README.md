# AHC054 — Treant's Forest

[Official problem](https://atcoder.jp/contests/ahc054/tasks/ahc054_a) ·
[Official generator and interactive tester, v2](https://img.atcoder.jp/ahc054/YDAxDRZr_v2.zip) ·
[Official ranked results](https://atcoder.jp/contests/ahc054/results/json)

Absolute score is the actual number of adventurer moves before reaching the
flower, larger better. For each of 3000 final cases, relative score is
`round(10^9 * own_moves / max_legal_moves_case)`; sum rounded scores across
cases. Illegal/timed-out cases get 0 and do not contribute to the maxima.
The official [score matrix](https://img.atcoder.jp/ahc_standings/ahc054/result.csv)
contains 1190 contestant rows. Taking the positive maximum of each column
and rounding separately with integer arithmetic reproduces the official
rank-1 user, saharan, with **2,409,003,089,874**. Matrix SHA-256:
`50617ebf3ae1d3a70dd27b4c712e61d72543b6a58ba7816e2fee6039dade0293`.
The [official input metadata](https://img.atcoder.jp/ahc_standings/ahc054/input.csv)
has SHA-256 `12a79763705f3118ee2f2ce80c50a6f0a3569e89b251f98b173bc338048d3ba9`
and columns file, seed, N and K; the statement also links the final
[seed list](https://img.atcoder.jp/ahc054/seeds.txt) with expected SHA-256
`f46e04db0ec8f80d641bbc8e166b1d2f4050a340b356acea1adce1a41a0a7fc8`.
No historical cases were used for either development experiment. See
the [separate frozen official audit](OFFICIAL_AUDIT.md) for the held final result. If the candidate improves any maxima, rerank the
historic winner and candidate on the same new maxima.

## Static maze construction

Control leaves the original forest untouched. Shortest-path confinement
checks that holding a legal path and blocking all other empties works, but
loses exploration. Candidate confines the board to a much longer induced
path. At the first turn only the entrance is revealed; after planting,
all moves are zero placements and we flush per protocol.

Maintain a candidate path prefix and exclude all earlier vertices and their
side neighbors from the future path. At a given endpoint, temporarily
exclude all its immediate options together, then BFS from the flower once.
An option whose neighbor has finite distance can be committed with
connectivity guaranteed; extending via another one of the options would
make a chord back to the old endpoint and is not allowed. Rank safe
steps by remaining distance, possible exits, heading and deterministic
noise. Use a mix of forward and backward starts and retain the longest
finished path, falling back to an initial legal BFS route. This is static
and uses no hidden destinations.

| Fresh cases | Untouched ↑ | Shortest-only ↑ | Induced ↑ | Induced vs untouched |
|---|---:|---:|---:|---:|
| Dev (24) | 72.292 | 22.542 | 489.042 | 24 / 0 / 0 |
| Holdout (24) | 56.042 | 22.542 | 404.375 | 24 / 0 / 0 |

All 144 runs were legal with exact score agreement between the official
tester and independent re-simulation of rays, revealed cells, target queue,
movement tie order, and true start-to-flower connectivity. On both path arms,
the moves equaled the actual path length in every tested case. Maximum
tester-inclusive wall time 0.104598 s (official 2 s includes tester).
The induced route is the branch default.

The repository's `grid-bfs.hpp` and `graph-bfs.hpp` return newly allocated
distance, parent and queue storage on each call, which did not suit repeated
changing reachability masks within each route. The new
[`MaskedBfsWorkspace`](../../library/masked-bfs-workspace.hpp) reuses them
and uses search stamps; test compares distances and parents with standard
queue BFS under 3000 changing masks.

**Next hypothesis:** retain some off-path blind spurs attached to exactly one
spine cell. Keep the entire spine and introduce no shortcut; possible
extra visits should only increase move count. Compare against spending
equivalent budget on more induced-path candidates. Visitation probability
can be estimated with independent fictional priority permutations, not the
official hidden target order. Avoid long artificially revealed spur
corridors that do not draw the adventurer beyond their attachment point.

```sh
p=problems/ahc054
d=$(mktemp -d)
for i in 0 1 2; do
  g++ -std=c++23 -O3 -DAHC054_SPURS=0 -DAHC054_PLAN=$i "$p/main.cpp" -o "$d/arm$i"
done
python "$p/benchmark.py" /path/to/gen /path/to/tester \
  "$d/arm0" "$d/arm1" "$d/arm2" "$d/results.csv"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/submission"
g++ -std=c++23 -O3 tests/masked_bfs_workspace_test.cpp -o "$d/bfs-test"
"$d/bfs-test"
```

## Follow-up: blind spurs

Keep exactly the same 30-restart spine and grow induced side paths of
length 4–15 with at least one bend. A new side-path cell must touch only
its immediate predecessor among all retained cells, so excursions have
one attachment and cannot shorten the path to the goal. The tester chooses
whether the adventurer explores a spur. No simulated hidden priorities were
used for candidate selection. The 60-restart spine-only control uses more
CPU, deliberately favoring the alternative explanation (more search).

| Fresh cases | Frozen spine ↑ | 60 starts ↑ | Blind spurs ↑ | Spurs vs 60 starts |
|---|---:|---:|---:|---:|
| Dev (24) | 469.542 | 477.125 | 1020.958 | 24 / 0 / 0 |
| Holdout (24) | 423.000 | 429.833 | 1047.333 | 23 / 1 / 0 |

Holdout median gain 201 moves versus 60 starts. Tester-inclusive median
0.034253 s for spurs vs 0.053301 s control, and maximum 0.230074 s across
all arms. Every run matched official and independent dynamic replay, and
the shortest start-to-flower path stayed identical to frozen for every
spur instance. Keep this as branch default. [Results](benchmark-spur-results.csv).
Next test one or two long blind branches grown before general filling, or
reserve forks for late placements that make previously attractive remote regions
unreachable without ever breaking the protected path.

```sh
p=problems/ahc054
d=$(mktemp -d)
for i in 0 1 2; do
  g++ -std=c++23 -O3 -DAHC054_SPURS=$i "$p/main.cpp" -o "$d/arm$i"
done
python "$p/benchmark_spurs.py" /path/to/gen /path/to/tester \
  "$d/arm0" "$d/arm1" "$d/arm2" "$d/results.csv"
```
