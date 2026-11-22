# Sparse one-ended reserve in the normal paired banks: live pilot (rejected)

The prebuilt feasibility model cleared its conditional passive throughput
screen; this is the single matched live check. Only M>=7 and the current
3×12 scaffold. On each bank the fixed checker pattern earmarks 4 of
12 tubes for a left/right-alternating prebuilt cap (12 total). Cap
only when all existing braces are already ready and body **empty**,
from the actual outside stance, after normal moves/builds with an
otherwise free player not holding a normal pair assignment. A free
worker already in that service lane can move at most two rows toward
the cap. If the other opening is being closed normally, no single cap.
After a cap, remaining legal capture needs one guard on its actual
open side. Ordinary paired legal captures and pair selection take
priority; eligible otherwise-free single guards use the same distance
shape/scoring structure and existing lane/half. Exterior dog fresh
pursuits are not considered for capped branches (model reasoning),
but actual legal captures of any species would be allowed. Same
release160/225, brace layout/logic, regular tactical fallback, no
M<7 behavior change. This differs from prior closing an occupied
tube early on one side while awaiting second crew.

| Used M>=7 dev30 | Narrow research default | Hybrid |
|---|---:|---:|
| Score | 575482884 | 535413192 |
| Direct paired / single exact-four animals | 201 / 0 | 207 / 6 |
| Direct total exact-four animals | 201 | 213 |
| Final exact-four animals | 207 | 218 |
| t160 residual | 271 | 258 |
| Final residual | 62 | 48 |
| All clear | 10 | 12 |
| Distinct human clean area sum | 4059 | 3104 |
| Pet-free humans | 105 | 101 |
| Brace-ready tubes t50, t101 | 688, 1052 | 688, 1049 |
| Caps by t50 / t101 / actual handoff | 0/0/0 | 5/38/139 (of 360) |
| Median first cap turn | not applicable | 90 |
| Observed legal/staffed single tube-turns | 0/0 | 81/6 |
| Distinct legal/staffed single animal IDs | 0/0 | 11/6 |
| Median/max sec | .379/.587 | .389/.571 |

W/L 11/19. True single captures: pig 1, rabbit 4, cat 1, cow/dog 0
(5 cases). All direct bank captures by species cow/pig/rabbit/dog/cat:
[36,30,29,62,44] → [31,36,29,68,49]. 79 distinct case/pet IDs caught
only by hybrid and 67 caught only by default in the bank stage; paired
same-input IDs are descriptive because paths and RNG use differ, not
direct causal capture/displacement labels. All 76 official/exact replay
valid, asserted one- or two-door closes yield exactly four cells and
the service exterior remains connected each turn. All 8 M<7 case full
bytes and geometry unchanged.

Single live staffing 6/81=7.4% conditional legal tube-turns, well below
the assumed single-duty .20–.50 range. The conditional prebuilt model
does not predict this live prep/crew behavior. Real compact capture and
all-clear increased, but clean area and score fell, so fails the
predeclared score gate. No holdout, new cap parameters, or promotion.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-capped.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-capped-hybrid.patch"
g++ -std=c++23 /tmp/ahc008-capped.cpp -o /tmp/ahc008-capped
g++ -std=c++23 "$D/parallel-banks-narrow-rescue-research.cpp" -o /tmp/ahc008-narrow
python "$D/parallel_banks_capped_hybrid_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-narrow /tmp/ahc008-capped "$WIDE_DEV/eligible" /tmp/caps.csv
python "$D/parallel_banks_capped_hybrid_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-narrow /tmp/ahc008-capped "$WIDE_DEV/low" /tmp/caps-id.csv --low-workers --case-count 8
```
