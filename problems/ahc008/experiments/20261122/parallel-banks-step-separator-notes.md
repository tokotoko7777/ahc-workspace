# At-handoff bank-anchored stepped separator feasibility (read only)

On the exact confirmed narrow rescue trajectories, freeze the actual
t160 or t225 handoff board. All 30 full score/action hashes and official
v3 plus independent action replay match. No modified agent commands.

Generate transverse vertex cuts with source/sink exterior row strips,
skipping the currently installed brace walls; within a width-seven
band the mincut may cross each bank at different rows instead of one
global straight line. Levels 8/12/16/20/24 in the bank's real orientation,
both sides, include the optional case of known visible opposite-side
animals forced outside. Primary mincut cost is missing wall count,
secondary graph distance to existing crew stances. Same snapshot
comparison with all four straight directions and legacy-style top4,
two rounds of shifting one to three consecutive cells in an otherwise
straight shape. The latter is a geometry analogue, not a byte-identical
replay of prior wiggle selection or priority/risk coefficients.

All candidates selected for comparison have <=18 unbuilt walls and are
currently legal against all observed human/pet occupancy. Require a
true connected retained side >=300, at least one actually separated
animal and local per-person projected score improvement. Open redundant
far-side cut cells not incident to the kept component (does not change
the kept flood), then reconstruct exact pet-only components. For routing
require that all humans can get into the retained part in <=6 steps
through real present graph, **before** assigning a static route to build
the remaining walls from that retained side within 12 total turns. The
scheduler uses explicit feasible stance paths and allows multiple jobs
per worker. It is a conservative constructive test, not an optimum;
preparatory jobs could legally overlap evacuation in some cases, and
static animal positions do not forecast future gate clearance.

| Used dev30 actual handoff | Legacy straight + wiggle family | Stepped |
|---|---:|---:|
| Unique positive (wall set, kept component) geometries | 727 | 164 |
| Cases with such geometry | 17 | 25 |
| Cases with sterile geometric side >=300 | 0 | 4 |
| Cases with constructive short route | 1 | 0 |

184 distinct short stepped masks before keep checks; the 164 qualifying
stepped compositions fail 58 at initial crew evacuation or 106 later
crew route stages. The unique legacy route is not necessarily sterile;
its replay witness is in the ledger. No case with an actually routed
novel stepped separator under these criteria.

Four geometry-only sterile witnesses (all t225, zero-based pet IDs):
527 cells/16 walls excluding cat #9 in a nonrectangular 141-cell
component (0009); 484/12 excluding pig #11 in nonrectangular 190 (0010);
528/14 excluding cow #8 in nonrectangular 144 (0013); 348/16 excluding
cow #6 in nonrectangular 326 (0026). The first three fail static crew
route; fourth fails initial evacuation. None is a compact pen or an
observed capture. The ledger stores both sides' relevant masks,
stance routes where feasible, animal IDs and counts, and the reason.

Synthetic fixture checks a one-opening wall creates a 419-cell retained
side with a real 3-turn route and that reopening a redundant far side
wall preserves exactly the same keep flood. Mincut construction is
separate from actual game legality. No live A/B or new seeds/thresholds.

```sh
D=problems/ahc008/experiments/20261122
g++ -std=c++23 "$D/parallel-banks-narrow-rescue-research.cpp" -o /tmp/narrow
python "$D/parallel_banks_step_separator_audit.py" "$HARNESS" "$TESTER" /tmp/narrow "$DEV30" "$D/parallel-banks-empty-guard-dev30.csv" /tmp/step-audit.json
```
Python uses NumPy and SciPy (the latter for integer vertex mincut).
