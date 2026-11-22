# Veto an exact empty ordinary local pocket closure (not adopted)

Control: confirmed current no-args narrow rescue, 2×13 iff M<=6.
Test one guard in `pocketPlan` after ordinary local build legality:
only consider the final missing planned wall after the normal rescue
handoff. Test exact component geometry with existing walls plus all
already-accepted same-turn ordinary walls and the candidate last wall.
Retain all preparatory work. Veto the last wall only when at least one
real new component is disconnected from the builder's outside stance,
**every** newly disconnected component is wholly inside the intended
local pocket, and none holds any observed animal, current human or
accepted human move destination. No route or priority changes, no bank,
wide-line or tactical terminal change. Static geometry `animal_in`
is not proof; the new gate checks the true current passable graph.
Retrospective practice 0089 is illustrative, not tuning or a benchmark case.

The overlay is off by default; benchmark flags `5 1 1 0` and `5 1 1 1`
match the current narrow selection and differ only by this veto. A three
case legal/exact-playback smoke preceded the old paired dev30.

| Used dev30, official exact | Narrow | Empty guard |
|---|---:|---:|
| Score | 746775771 | 739241238 |
| All-clear cases | 18 | 15 |
| Final residual | 42 | 45 |
| Distinct human-safe area total | 5884 | 5820 |
| Cleared people | 136 | 129 |
| Direct bank exact-four captures | 206 | 206 |
| Final exact-four-pen animals | 212 | 213 |
| Post-release nonrectangular 4–48 captures | 53 | 46 |
| Actual newly detached empty components after handoff | 108 | 85 |
| Actual total cells lost to those empty components | 372 | 225 |
| Median/max official wall seconds | .331/.443 | .352/.463 |

W/T/L 6/17/7, full transcripts identical 17/30. All 60 legal official and
independent replay; exact shared prefix for every case through its
unchanged handoff. Both immediate bank correctness and actual component
counts use pre-pet-move and post-wall geometry. Empty totals count all
ordinary/tactical output (observable behavior), rather than assuming
every empty piece was built by the local planner. Overlay applied 194
exact checks and refused 167 individual proposals (and repeatedly
refused some gates, so the count is not 167 distinct saved pockets).
Preserved local area did not improve observed final safe area or reliable
capture. Tradeoff likely includes plan persistence/retries but this does
not authorize an additional tuning round. No promotion/unseen retest.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-empty-guard.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-empty-guard.patch"
g++ -std=c++23 /tmp/ahc008-empty-guard.cpp -o /tmp/ahc008-empty-guard
python "$D/parallel_banks_empty_guard_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-empty-guard "$DEV30" /tmp/empty.csv
```
