# Prepare one usable five-tube contiguous starter block

A separate construction-only comparison against unchanged staged+zones
on the old dev30; not against the failed whole-snake variant. Initial
bank geometry, staged guard activation and ranking, atomic closure, and
t225 fallback fixed. Select one contiguous five-tube block (six brace
rows, 24 wall cells) and plan three paired-above/below sweeps. Search the
two directions and every split among 12 stance tasks with two distinct
crew, using actual shortest endpoints in final brace-only passable
graph. Rank shorter nominal makespan with small preference for nearby
visible initial pets. Before all 24 walls or t55, those two people work
only on the starter block while the rest keep greedy disjoint braces.
Hunting retains priority, potentially removing a preassigned builder
once a physically available bank gets staffed. Legal blockers skip
a stance; the same nearby assigned workers prioritize local repair.

All 60 official/legal and independently exact replay, same geometry
and actual humans/aisles connected through t225. Nominal median25,
the starter block completely ready 29/30 (conditional median29).
Mean starter walls installed23.6/24 at t35 and23.87 at t50. This
does not translate to materially better *actual* early capture.

| Metric old dev30 | Greedy | Starter block |
|---|---:|---:|
| Mean whole tubes ready t35 | 13.33 | 13.93 |
| Mean whole tubes ready t50 | 20.20 | 19.97 |
| Mean whole tubes ready t101 | 32.33 | 31.70 |
| Mean whole tubes ready t225 | 33.07 | 32.47 |
| Median first bank≥5 (no earlier than t35) | 35 | 35 |
| Median first actual seal | 58 | 56 |
| True four-cell captured t<35 / t<50 | 0 / 10 | 0 / 11 |
| Total direct exact-four pets | 270 | 283 |
| Species cow/pig/rabbit/dog/cat | 29/42/54/83/62 | 33/51/49/84/66 |
| Release residual → final | 165→116 | 152→96 |
| Final pet-free human union (case sums) | 4144 | 1661 |
| All-clear cases | 9 | 3 |
| Score | 541,987,952 | 547,579,799 |
| Max tester wall seconds | .641 | .749 |

Paired capture 14/4/12 and score19/0/11 (wins/ties/losses).
One more early capture, narrower late completion, lower residual, but
six fewer all-clear cases and far less total safe area. Reject promotion
because isolated already-used development benefits are mixed and true
early benefit is modest. No thresholds tuned, no holdout.

Apply pilot/staged/zone then parallel-banks-starter.patch to a scratch
tactical source; GNU C++23. CLI 5 0 baseline and 5 1 starter. Run
parallel_banks_starter_bench.py with harness, tester, tactical binary,
new dual-mode binary, old dev30 input, output CSV.
