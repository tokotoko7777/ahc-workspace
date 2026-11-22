# Group movement: occupancy and sector-flow audit

Use the same capture4 solver, 30 development cases and official v3 verified
recorded actions as the collective-cut and baffle audits. In these simulations
we did not change any solver actions. The seed file is archived in order.
There were no new live candidate or distributed score comparisons.

## What prevents a big clean component?

Enumerate 1,976 connected row-/column-convex unions of macro rooms,
incorporate internal portals and remove existing physical wall cells.
Find exact connected components of size ≥300 and ≥450 at the 18 sampled
turns 40–210. Choose minima by instantaneous animal count, breaking witness
ties by larger area then earlier turn. No assumption that boundary walls are
legal or staffable. Counterfactual removal means ignoring a species' existing
positions for occupancy only; it is not altered movement simulation.

| Threshold | Naturally clear | Ignore cow | Ignore pig | Ignore rabbit | Ignore dog | Ignore cat | Ignore dog and cat |
|---|---:|---:|---:|---:|---:|---:|---:|
| 300 | 14/30 | 21 | 25 | 21 | 22 | 19 | 25 |
| 450 | 1/30 | 4 | 5 | 8 | 2 | 6 | 11 |

At 300 the remaining cases need one (11) or two (5) blocker relocations;
mean 0.70. At 450 it's six at one, five at two and 18 at three-plus;
mean 3.10. Even after ignoring dogs and cats 9 cases need one other
blocker and 10 need two or more. Species can differ but no single species
generally opens 450 cells. The 450 witnesses' 93 residual blockers include
24 cows, 19 pigs, 16 rabbits, 10 dogs, and 24 cats. For the fixed witnessed
450 regions 15 eventually appear outside over the following 20 observed
turns, versus 109 new in-region outsiders. The witness is hindsight.

## Causal sector selection, retrospective movement diagnosis

Separately, at each of t70/100/130 choose one ≥300-cell connected sector
using current information only: require ≤12 pending boundary cells;
minimize actual pet count, then pending boundary cells, then maximize area.
There are 81 valid sector-times out of 90 (27 at each turn); these are repeated
cases, not 81 independent samples. Hold the selected footprint fixed for
20 turns, reconstructing every pet movement substep. Evaluate whether an
observed incoming pet has a gate on its first path that could have been
legally closed by that move, with all observed outgoing paths of the
original blockers still open for the full horizon. Separately require
an actual in-sector guard at a neighboring cell, or reachable within two
moves. This close/no-close diagnostic is hindsight over unchanged outcomes,
not a causal decision rule, no guarantee other gates or humans are jointly
schedulable, and no claim that changed paths would not route around the wall.

- Initially 160 pets inside; 46 cross outside (partly reversible).
- 192 new incoming pets across 81 snapshots; 54 individually legal to
  block, 14 with a worker within two moves, 7 with worker already on stance.
- All initial blockers end out in only 7 of 64 initially dirty sectors.
  All observed initial ingress individually blockable in three of these
  cases, two with actual staff.
- Simple waiting or adding guards to partially open sectors does not address
  the failure. A genuine next architecture has to change crossings early:
  few deliberate exits for a broad sector, heavy staffing-control cost up
  front, and a way to bias or force more than dog motion. Keep the existing
  capture4 fallback and do not promote an exit-control policy as-is.

Reproduce (HARNESS contains benchmark.py and benchmark_cases.py):

    d=problems/ahc008/experiments/20261122
    g++ -std=c++23 problems/ahc008/capture_first_submission.cpp -o cap4
    python "$d/species_blocker_audit.py" "$HARNESS" "$TESTER" ./cap4 "$CASES" > species.csv
    python "$d/sector_flow_audit.py" "$HARNESS" "$TESTER" ./cap4 "$CASES" > flows.csv
