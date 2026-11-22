# S-shaped exact-four banks with rescue (development; not adopted)

Only M>=7 is experimental. Four banks by nine S tetrominoes over five
aisles at columns 2,8,14,20,26 (or transposed). For row r=2,5,...,26 and
bank start a, the pen body is {(r,a+2),(r,a+3),(r+1,a+3),(r+1,a+4)}.
Seal (r,a+1) and (r+1,a+5) in one legal turn from actual exterior
stances (r,a) and (r+1,a+6). Shape is a four-cell path, not a straight
tube or an ordinary later cut. The exact grid boundary less the two
mouths becomes shared brace cells (184 total unique targets). Open end
crossbars keep everyone connected throughout bank activity. Stable zones
use each actual stance row, including the right-hand row offset.
Same 36 pen count, local 5-ready/global 24-ready staffing, t101 hard
staffing and exact same t160 rescue rule. Two/four extra paired zone
guards as in the four-bank straight design where worker counts allow.
For M<=6 keep the confirmed 2×13 narrow rescue default. No new forecast,
mirror tuning, wider fallback, judge submission or main changes.

| Used common M>=7 dev30 | Narrow default (3×12 straight in this group) | 4×9 S |
|---|---:|---:|
| Official score | 575482884 | 490823827 |
| Direct exact-four animals | 201 | 199 |
| Final exact-four animals | 207 | 201 |
| All clear | 10 | 8 |
| Final residual | 62 | 57 |
| Distinct clean human area | 4059 | 2795 |
| Reachable residual at t160 | 271 | 274 |
| Actual handoff residual | 269 | 271 |
| Reachable area sum at handoff | 21164 | 20600 |
| Handoff t160/t225 counts | 29/1 | 27/3 |
| Tubes ready at t50 | 688 | 586 |
| Tubes ready at t101 | 1052 | 956 |
| Tubes ready at handoff | 1070 | 974 |
| Median first five-ready turn | 35 | 35 |
| Median first direct seal turn | 60.5 | 59.5 |
| Ready body event tube-turns | 5493 | 4875 |
| Staffed legal window tube-turns | 197 | 194 |
| Unique legal-window animals | 384 | 393 |
| Median/max wall seconds | .403/.604 | .439/.696 |

W/L 13/17. All 76 legal official runs matched independent exact replay;
all eight M<7 geometry and full action traces identical. The 199
direct S-pen captures are exact flood-verified with two same-turn seals:
40 cows, 27 pigs, 29 rabbits, 63 dogs, 40 cats. Capture by bank-stage
50-turn bins: [9,65,102,25,0] → [6,72,107,13,1]. Example 0000 t104 dog
#5 with resulting four cells (22,14),(23,14),(23,15),(24,15); simultaneous
gates (21,14),(25,15), all indices zero-based. Witness JSON has exact
action digest. Earlier first closure was not the bottleneck in this
sample; overall prepared inventory dropped and end clean area suffered
despite slightly fewer uncaught animals. No holdout; no promotion.
A single separate S3 implementation can probe the density/retained area
tradeoff without changing thresholds or introducing new geometry.

Reproduce with the earlier screened wide dev inputs (seed file
`parallel-banks-wide-enriched-seeds.txt`) and official v3 tester:
```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-bent.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-bent-rescue.patch"
g++ -std=c++23 /tmp/ahc008-bent.cpp -o /tmp/ahc008-bent
g++ -std=c++23 "$D/parallel-banks-narrow-rescue-research.cpp" -o /tmp/ahc008-narrow
python "$D/parallel_banks_bent_rescue_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-narrow /tmp/ahc008-bent "$WIDE_DEV/eligible" /tmp/s4.csv
python "$D/parallel_banks_bent_rescue_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-narrow /tmp/ahc008-bent "$WIDE_DEV/low" /tmp/s4-id.csv --low-workers --case-count 8
```
