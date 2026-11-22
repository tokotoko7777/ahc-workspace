# Independent permanent bank pairs, M=7 only (rejected)

At t101 assign one fixed pair to each of the three original exact-four
banks. Six stance jobs take the available prepared open row nearest
14 in their bank and are assigned by exact minimum sum of current BFS
ETAs; the remaining worker has the same ordinary construction role.
Shared aisles get one worker per neighboring side, not longitudinal
halves; both workers of one pair jointly target the same tube row
with the unchanged valuation and persistence. Ordinary any-worker
legal immediate paired closures always precede pursuit. No change to
geometry, readiness triggers, bank t0–100, rescue predicate (t160
residual >4), t225 normal maximum, or outside tactics. No-args keeps
the current narrow default; flags `5 1 1 1` activate only at M=7.

Cohort: first thirty M7 and first eight others selected by visible M
from an independent generated seed stream, no previously reported
validation cohort, no case result filter. Extended from 260 to 390
by the same sequential hash recipe before any benchmark because the
first segment contained only 23 qualifying M7 inputs.

| Independent development M7 (n=30) | Narrow | Independent bank pairs |
|---|---:|---:|
| Official score | 655321494 | 568454043 |
| Direct exact-four animals | 206 | 214 |
| Final exact-four animals | 212 | 215 |
| Staffed legal motif-turns | 198 | 204 |
| Other legal motif-turns | 1505 | 1463 |
| Unique legal motif-window animals | 368 | 373 |
| Residual at t160 | 251 | 243 |
| Actual handoff residual | 248 | 240 |
| Final residual | 60 | 74 |
| Fully clear cases | 13 | 7 |
| Pet-free people | 91 | 59 |
| Sum of distinct safe area | 3846 | 2211 |
| Median/max tester wall seconds | .377/.672 | .405/.580 |

W/L 11/19. Direct capture by species cow/pig/rabbit/dog/cat
[31,30,28,72,45] → [32,29,34,79,40]. Direct capture by 50-turn bins
[11,84,89,21,1] → [11,84,102,16,1]. Readiness at t50/t101 and median
first actual capture unchanged. All 76 legal and exact replay, all
t0–100 inclusive prefix bytes match, complete byte/geometry identity
on all eight inactive controls. Higher bank compact count and small
staffing gain did not translate into safe territory or final score.
No held-out retest, promotion, submission or main changes.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/m7.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-m7-pairs.patch"
g++ -std=c++23 /tmp/m7.cpp -o /tmp/m7
g++ -std=c++23 "$D/parallel-banks-narrow-rescue-research.cpp" -o /tmp/narrow
python "$D/parallel_banks_m7_pairs_bench.py" "$HARNESS" "$TESTER" /tmp/narrow /tmp/m7 "$M7_DEV/eligible" /tmp/m7.csv
python "$D/parallel_banks_m7_pairs_bench.py" "$HARNESS" "$TESTER" /tmp/narrow /tmp/m7 "$M7_DEV/identity" /tmp/m7id.csv --inactive --case-count 8
```
Use official generator with archived seed list; first 30 M7 and first 8
other M in filename sort order populate the two directories.
