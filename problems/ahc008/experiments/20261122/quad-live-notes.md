# Early exact-four-cell planned shells: matched live pilot

This pilot tested one chosen four-cell compact pen before returning to the
same tactical baseline. It is not the older human-refuge objective. The
three alternatives are 1×4 line, 2×2 square, and bent L tetromino. For each
family, enumerate all reflections/rotations touching the outer boundary
and all supported final gate choices. Using actual board-edge support, a
shape is sealed iff every physical boundary cell is built; all interior
cells remain connected and free. Require legal closure-safe cells of
both checkerboard colors, and a guard stance reachable from the outside.
Avoid initial humans in the planned pen or its walls.

Choose geometry once per case using current initial animal coordinates,
expected local mass under a simple open-board random-walk prior, and
estimated worker route/shoulder effort. Cow/pig/rabbit use 1/2/3 passive
steps; dogs and cats use a 2-step approximate prior here, not their real
official directed motion model. One gate builder and two designated
shoulder builders are assigned before setup. A pilot worker builds only
from outside; travel is recomputed through actual wall state and only
proposed actions permitted by current pet/human reach are used. Unassigned
human jobs continue except boundary/use conflicts. Setup aborts at t36;
after preparation watch for at most 27 turns, or until t68 overall. No
geometry-specific parameter tuning. The selected gate need not be at the
end of each 1×4; the policy evaluates feasible opening orientations for
every family, so this is not an end-vs-side-only controlled bench.

All four arms use the 30 common development seeds in the adjacent
collective-capture-dev30-seeds.txt. Every action sequence is driven by
official tester v3, then replayed byte-identically with independent score
and legality verification. Per-case CSV carries exact official sums,
runtime (including logging for pilot), hash, and capture/worker metrics.
There is no unused holdout or official distributed rerun.

| Metric | Tactical control | 1×4 | 2×2 | L |
|---|---:|---:|---:|---:|
| Shells planned | n/a | 30 | 30 | 30 |
| Shells completed | n/a | 30 | 30 | 29 |
| Completion median turn | n/a | 14.5 | 17 | 18 |
| Shoulder walls actually built | n/a | 147 | 142 | 180 |
| Distinct pets observed inside fully prepared shell | n/a | 6 | 15 | 14 |
| Distinct phase-end arrivals from outside on/after readiness | n/a | 4 | 8 | 8 |
| Fully prepared staffed turn count | n/a | 687 | 599 | 564 |
| Legal occupied guard turn count | n/a | 5 | 9 | 9 |
| Pilot sealed pens | n/a | 5 | 9 | 9 |
| Pilot exact-four-cell pets at seal | n/a | 5 | 10 | 9 |
| Final exact-four-cell pets across all pens | 4 | 5 | 10 | 10 |
| Final reachable pets, mean per case | 5.50 | 4.80 | 4.83 | 5.03 |
| Final all-clear cases | 4 | 5 | 5 | 6 |
| Assigned shoulder worker moves / waits | n/a | 378 / 65 | 570 / 65 | 550 / 98 |
| Assigned guard moves / waits | n/a | 327 / 748 | 375 / 707 | 317 / 738 |
| Diverted ordinary commands | n/a | 1,544 | 1,994 | 2,109 |
| Replaced ordinary wall commands | n/a | 98 | 189 | 289 |
| Official score sum, dev30 | 603,294,259 | 522,449,976 | 485,985,124 | 539,441,627 |
| Maximum tester wall seconds | .484 | .484 | .476 | .684 |

| Species | Observed inside line / square / L | Actual pilot captures line / square / L |
|---|---:|---:|
| Cow | 3 / 3 / 4 | 2 / 3 / 4 |
| Pig | 0 / 3 / 1 | 0 / 2 / 1 |
| Rabbit | 3 / 6 / 5 | 3 / 5 / 4 |
| Dog | 0 / 3 / 4 | 0 / 0 / 0 |
| Cat | 0 / 0 / 0 | 0 / 0 / 0 |

Square/L earn more seals than thin in this specific one-mouth policy
despite later completion. Their arrival opportunity was larger, not
merely shape-dependent conditional probability after arrival. The line
does not dominate by easier/faster preparation. Each family chose its
own highest-ranked site and gate, so these are matched policies on matched
inputs, not a same-site intervention controlling animal start relative to
mouth, travel, or local orientation. Dogs/cats can be placed
in compact cells under a controlled geometry fixture, but this operating
policy did not capture any. Do not report broader species impossibility.
No topology was promoted: all lose score, remain far from full capture,
and spend substantial worker time watching. On final exact-four pet count
the paired case wins/losses are line 5/4, square 9/4, L 10/4. Exactly five
square and five L successes close on the first fully prepared turn; later
square successes are +1/+2/+12/+13, and L +2/+2/+3/+8. A separately frozen
brief watch or strategy that releases people after near-term opportunities
expire is a plausible next efficiency test, but current sample is not
evidence of resulting official score.

Reproduce in scratch (apply patch only to the scratch copy or apply in an
isolated worktree), using official tester v3 and repo benchmark wrappers:

    d=problems/ahc008/experiments/20261122
    g++ -std=c++23 problems/ahc008/tactical_gate_submission.cpp -o tactical
    patch -p0 < "$d/quad-live-pilot.patch"
    g++ -std=c++23 problems/ahc008/tactical_gate_submission.cpp -o quad-pilot
    python "$d/quad_live_bench.py" "$HARNESS" "$TESTER" ./tactical ./quad-pilot "$CASES" compare.csv

The standalone patch inherits the real source GCC optimization pragma and
uses fast I/O. The pilot's argument selects shape (1/2/3); the benchmark
uses a separate sidecar file per invocation for logging, not a hidden
game signal. Cumulative capture counts verify exact connected area from
recorded post-seal board states, not solver assertions of intent.
