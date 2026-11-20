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

## Follow-up: first-placement-conditioned forecast

Both arms preserve the same original fallback, then evaluate the same
eight long-translation walks with the same acceptance/tie rules. For each
first action and each of its three second actions, the new arm evaluates
the second action's event improvement against the exact minimum of
the current distance field and distance from the first action's ward.
This avoids double credit where both actions reach the same near-future
attack. The unconditioned control uses the same probes and horizon.

| Fresh cases | Approx. danger ↓ | Conditional danger ↓ | Conditional win / tie / loss |
|---|---:|---:|---:|
| Dev (24) | 351,711.79 | 350,302.46 | 16 / 0 / 8 |
| Holdout (24) | 351,640.50 | 351,747.54 | 12 / 0 / 12 |

Holdout mean score 2,843,945 vs 2,843,117, median danger saving −72 and
slightly lower unique coverage for conditional. Both medians .033 s.
The gain does not generalize; retain the prior default, alternative opt-in.
All 96 legal and exact official/independent; max wall .062479 s.
[Logs](benchmark-marginal-results.csv). No historical case use.

Next replace per-turn greedy with periodic finite-horizon route patches:
freeze the same translations, mutate a short action window while forcing
the same endpoint (permuting the same multiset of translations), and
rescore earliest-placement changes for only nearby future event cells.
Compare against equal-work independent legal rollouts.

```sh
p=problems/ahc070
d=$(mktemp -d)
for mode in 0 1; do
  g++ -std=c++23 -O3 -DAHC070_CONDITIONAL=$mode "$p/main.cpp" -o "$d/arm$mode"
done
python "$p/benchmark_marginal.py" /path/to/gen /path/to/vis \
  "$d/arm0" "$d/arm1" "$d/results.csv"
```
