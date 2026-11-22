# AHC008: why the last enclosure gets abandoned

This checkpoint uses frozen v24 with its ordinary two-ended 1×4 geometry.
It is not a reproduction or test of TERRY's original Rust submission.

A diagnostic recorded the last-1-or-2 planner's decisions on all 30 fixed
five-worker development cases. All action hashes and official scores match
the historical independently replayed traces. Of 20 non-full outcomes,
13 invoked that planner at least once, eight by turn 275. Across the nine
non-full cases that adopted a plan, 51 plans were started and invalidated.
Only one closing command was recorded. In 42 of the 51 invalidations, a
reachable animal was standing on a planned wall cell; in eight it was
outside the prospective trap, and in one it was inside but off planned
walls. This classifies observed states, not whether waiting would help.

## Separate changes

1. Permit continued routing and unfinished safe work when an animal occupies
   a proposed wall of an already selected enclosure. The physical wall still
   cannot be built while an animal blocks it, and initial shape selection,
   legality and abandonment after a full escape remain unchanged.
   Same dev30: 434,537,087 → 435,481,529; whole 10 → 10; useful whole 2 → 2;
   remaining 62 → 62. No promotion.
2. Start again from v24. For candidate enclosures of at most five walls
   and five workers, calculate the fastest simultaneous completion over
   distinct workers using exact exterior distances and bitmask assignment.
   Penalize assembly time beyond the estimated time before an animal can
   enter a gate's adjacent neighborhood. The estimate is a ranking proxy,
   not a forecast or a guarantee. This modifies only geometry selection in
   the late fallback, not physical legality or early 1×4 capture.
   Same dev30: 434,537,087 → 478,231,531; whole 10 → 9; useful whole 2 → 3;
   remaining 62 → 62. Use the preregistered independent M=5 100 before
   deciding; `makespan/HOLDOUT.md` records the gate.

## Reproducibility

Use the [pinned standalone v24 C++23](../v24-source-release/README.md).
Each `make.py v24.cpp output.cpp` generates a single file; build with
`g++ -std=gnu++23 output.cpp -o solve`. The [same harness and fixed
development 30](../v24-five-worker-rescue/README.md) apply here. Use
`holdout/generate.py OFFICIAL_GEN` with the seed list decompressed or generate
the exact 100 inputs from the ordered manifest and check the input hashes.
`reason/audit.py` is original workspace tooling: it retains original relative
paths and baselines, while `reason/result.json.gz` is the verified record.
Useful whole means at least 300 mean human-reachable cells at state 300.

## Held confirmation of the distinct-worker ranking

All 200 runs on the new stratified five-worker set passed the official judge
and exact independent replay, and every input hash matched the prior manifest.
No seed in this set had been used to choose the policy. v24 → candidate:

| Metric | v24 | Candidate |
|---|---:|---:|
| Score | 1,291,647,901 | 1,317,286,791 |
| Whole clears | 24 | 23 |
| Whole clears with ≥300 cells | 6 | 5 |
| Final accessible animals | 272 | 270 |
| Maximum local judge wall clock | 0.642s | 0.589s |

18 action histories changed, with three new whole clears and four losses.
The independently fixed acceptance screen required the full-clear metrics
not to regress, so this was not promoted. Do not tune against these cases.
The next hypothesis is explicit pet location forecasting at the time the
last wall can be executed, rather than the pessimistic gate-distance proxy.

## Static forecast ablation

Starting again from v24, score each prospective finish using 24 shared animal
paths from the current board. On the estimated closure turn, include only
samples in which all required wall cells and their neighbors are unoccupied
by animals, then average the retained-area gain from isolating sampled pets.
Blend 10% of the current-position score. Physical commands still recheck
actual legality. This forecast freezes wall and worker positions and does
not model partial wall construction or worker-attracted animal trajectories.
Dev30 (all 60 official and independently replayed): 434,537,087 → 436,064,865;
whole 10 → 11; useful whole 2 → 2; final remaining 62 → 61. No adoption and
no evaluation on the previous holdout. Next test, if helpful, use staged
construction in the forward simulation with a small CPU limit, not further
tuning of a static ranking score.
