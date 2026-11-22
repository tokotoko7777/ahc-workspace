# Smaller S tetromino scaffold (not adopted)

The earlier 4×9 S experiment had a normal first-seal time but fewer
ready tubes at t50/t101 and substantially smaller human clean region
in the end, despite slightly fewer residual pets. Test exactly one
reduction: three banks x nine of the identical S tetromino, with fixed
aisles 5,11,17,23 (or transposed), 138 shared braces, and only when
M>=7. With the restored four aisle roster use the original 3-bank
stable zone policy, each guard's own actual stance row; local 5 and
global 18/27 keep the same expansion proportions. No change to the
t160 observed residual trigger, ordinary closing priority, rescue
tactics or M<=6 policy.

| Same used M>=7 dev30 | Straight3×12 | S4×9 (previous) | S3×9 |
|---|---:|---:|---:|
| Score | 575482884 | 490823827 | 421722487 |
| Direct actual exact four | 201 | 199 | 185 |
| Final exact-four animals | 207 | 201 | 189 |
| Final residual | 62 | 57 | 64 |
| All clear | 10 | 8 | 8 |
| Distinct human clean area sum | 4059 | 2795 | 2089 |
| Covered eligible pets | 384 | 393 | 350 |
| Staffed legal tube-turns | 197 | 194 | 179 |
| Ready at turn 50 | 688 | 586 | 560 |
| Ready at turn 101 | 1052/1080 | 956/1080 | 796/810 |
| Handoff usable area sum | 21164 | 20600 | 21787 |
| Reachable residual t160/actual handoff | 271/269 | 274/271 | 286/285 |
| Late (225) handoff cases | 1 | 3 | 1 |

S3 W/L 14/16 versus straight. Median first true simultaneous capture
t62.5, compared with 60.5 (straight) and 59.5 (S4). Direct S3 captured
24 cows, 23 pigs, 28 rabbits, 64 dogs, and 46 cats; the exact replay
checks that the entire four-cell nonrectangular body alone is reachable
from each animal after the two gate commands. All 76 successful official
and independently exact replay; low-worker eight compare full geometry
and full action bytes identical. Max tester seconds .556→.671 in the
freshly paired S3 timing; S4 timing is in its own run.

The smaller scaffold restores human-reachable area at handoff but not
enough covered legal arrivals, capture count, or final sterile area.
This rejects this one smaller macro layout, not the feasibility of
irregular four-cell capture. No unseen100, promotion or other tuning.
Score-first official research default remains the narrow-rescue standalone.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-bent3.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-bent3-rescue.patch"
g++ -std=c++23 /tmp/ahc008-bent3.cpp -o /tmp/ahc008-bent3
g++ -std=c++23 "$D/parallel-banks-narrow-rescue-research.cpp" -o /tmp/ahc008-narrow
python "$D/parallel_banks_bent3_rescue_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-narrow /tmp/ahc008-bent3 "$WIDE_DEV/eligible" /tmp/s3.csv
python "$D/parallel_banks_bent3_rescue_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-narrow /tmp/ahc008-bent3 "$WIDE_DEV/low" /tmp/s3-id.csv --low-workers --case-count 8
```
