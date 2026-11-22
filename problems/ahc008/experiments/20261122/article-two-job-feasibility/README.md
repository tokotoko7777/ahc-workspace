# Two-job primary crews: exact constructive timing, no live action

Read-only successor to v6, on identical 19 first eligible states in old
legal v4 dev30. Uses the same small target-connected min-cut enumeration
and all-human exterior certificate. Retains 4–48 actual nonrectangular
selected animal compartment, and checks no separate large animal pen.
Consider at most twice the actual primary crew's size in pending walls
instead of once. For each worker and unordered one/two-job subset,
shortest paths choose the best possible exterior stances and both
orders. A disjoint wall-mask minmax DP produces the optimum of
those ≤2-job schedules. If a person has two jobs, they place their
first wall en route, never entering the cut or pet territory. Final
jobs then finish atomically when all end positions can be occupied.
Shared positions are allowed. At fixed walls those walks are legal
by construction; real animal adjacency at a future build time
can still invalidate the schedule.

All 30 unchanged v4 official action hashes/scores and independent
playbacks match. Generation gave 14 real 4–48 cuts with more jobs than
crew across nine cases. All fit before t205 in the static route model:
11–29 turns, with 11/14 prospective cut cells all unblocked at the
initial snapshot. On the unchanged factual animal tape (NOT modeled
post-intervention trajectories), three target observations remain inside
at the projected last wall, none never left the intended pen, and none
satisfy target inside AND all scheduled wall legalities. Every selected
target was a cow, pig or rabbit. The model could have altered these
animal paths by constructing earlier walls; it neither predicts nor
guarantees their factual capture. No live arm due conditional feasibility.

Restore v6 candidate source (v4 standalone + v6 patch), run
`python make_audit.py v6.cpp two_routes.hpp audit.cpp`, compile GNU C++23
`g++ -std=c++23 audit.cpp -o audit`, run
`python check_audit.py HARNESS TESTER audit DEV30 V6.dev30.csv events.jsonl summary.json`.
The audited candidate intentionally issues only unchanged v4 actions.
Unit for helper: supply v6 header (v1 + v4 + v6 header patches), build
`g++ -std=c++23 two-routes-unit.cpp -o unit` and run. Verified 2000
random subset oracles and real 3-wall/2-person shortest stance routes;
independent Python additionally verifies every logged dispatch's
distinct jobs, stance adjacency/BFS, static blockers and actual pen.

## Exact passive conditional follow-up

Estimated only the 14 previously enumerated fixed wall schedules, using
verified t160 snapshots from nine official baseline replays. No original
future moves enter the model, and only official independent uniform C/P/R
substeps are used. At every scheduled human-build phase separately kill
animal states in the target or orthogonal vicinity of any legal requirement,
insert scheduled walls before animal substeps, and at final atomic time
require selected animal inside the genuine target component. Other
passive animals' legal-survival factors multiply because the fixed wall
schedule gives independent transitions. Ignore dog/cat block probability,
additional unsimulated secondary walls and any live adaptive timing.
This gives an upper bound for the specified simplified fixed plan with
directed blockers reintroduced, not a bound on another adaptive policy
or on actual judge RNG. Also compare target-only occupation at the same
finish horizon without adding any scheduled wall.

Only one of 14 increases target-in-pen-and-own-legality probability vs
untouched target occupation; mean .0990 vs .2468. Optimistic expected
success across nine cases is .440 when allowed the best pre-enumerated
shape per case; best individual .1676 and next .1323. Three cases exactly
zero. Thus even incorporating the causal effects of this schedule on
passive walks offers little. No live variant. Same no-activation
reconstruction and general optimizer remain archived above.

Run `passive_exact.py HARNESS TESTER V4 DEV30 V4_REF.csv
two-route-dev30.jsonl result.jsonl summary.json` with NumPy available.
This includes a simple legal-state-elimination/isolated-stay fixture.
