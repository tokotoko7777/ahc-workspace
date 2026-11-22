# Paired-row brace work using exact snake assignment

Separate construction-only test on the same stable-zoned 3×12 body-only
bank and used dev30. The geometry, staged staffing rule, capture ranking,
immediate legal two-gate closure and t225 cleanup are unchanged; no
direction cue and no irregular overlay. [TERRY's authorized write-up](https://blog.terry-u16.net/entry/ahc008)
motivates assigning contiguous portions of a fixed construction walk.
The implementation uses the team's independent exact route minimax kernel,
not contest source code.

Choose from 16 snake variants. Each stand in one open tube row builds its
brace cells directly above and below. Adjacent sweeps are connected by
shortest paths in the final brace-only passable graph; the worker's
initial cell may be one of the designated wall cells. Minimize maximum
planned construction approach plus work plus travel by exact worker-mask
assignment and split the stance sequence into continuous portions. At
execution, current walls including actual closures can modify walking,
so re-evaluate route moves. If an adjacent brace build is blocked by an
observed animal/human, build the other legal adjacent half when available,
otherwise skip the stand. Workers committed to hunting do not continue
their prescribed construction route at that time, and unassigned workers
fix leftovers using the original greedy jobs. No simulation of pet
movement in the solver's initialization lower bound.

All 60 legal and independently exact official replays, geometry
identical on every pair; each actor/service lane connected until t225.
The proposed no-block makespan is median50 / mean55.8. Actual counts:

| Dev30 measure | Greedy setup | Snake-route setup |
|---|---:|---:|
| Mean brace cells already built t35 | 88.00 | 88.13 |
| t50 | 108.73 | 124.80 |
| t75 | 133.07 | 145.13 |
| t101 | 145.23 | 150.77 |
| t225 | 147.73 | 152.43 |
| Mean *complete recurring tubes* t35 | 13.33 | 7.03 |
| t50 | 20.20 | 17.50 |
| t75 | 28.40 | 28.63 |
| t101 | 32.33 | 33.00 |
| t225 | 33.07 | 33.93 |
| Median first bank with 5 ready tubes (≥t35) | 35 | 42 |
| Median first seal | 58 | 76 |
| Direct exact-four pets | 270 | 271 |
| Banked species cow/pig/rabbit/dog/cat | 29/42/54/83/62 | 27/51/50/79/64 |
| Actual captures by 50-turn blocks | 10/68/96/71/25 | 5/49/114/82/21 |
| Residual at t225 → final | 165→116 | 164→131 |
| All-clear cases | 9 | 5 |
| Total distinct final pet-free human union | 4144 | 2507 |
| Official score | 541,987,952 | 431,412,787 |
| Max tester seconds | .603 | .482 |

Routed output proposed 3,943 wall commands, 6,352 walking commands and
377 stand skips (not credited as successful builds; official traces
determine actual installed walls). Paired final compact W/T/L 11/8/11,
score15/0/15 with larger losses. Reject scheduling all walls for makespan
alone. It improves total wall installation but leaves fewer whole tubes
available when the first guard pair can exploit them. A genuinely
different next scheduler would prioritize connected early usable batches
and measure activation and retained coverage, not just weighted wall counts.

Apply parallel-banks-pilot.patch, parallel-banks-staged.patch,
parallel-banks-zone.patch and parallel-banks-route.patch to a scratch
tactical source, compile GNU C++23. CLI 5 0=old, 5 1=snake.
Run parallel_banks_route_bench.py with current harness directory, tester,
control executable, new executable, old capture-dev30 inputs, output CSV.
