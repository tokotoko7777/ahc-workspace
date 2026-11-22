# Orthogonal x7-20-13 paired banks (rejected score arm)

Only M>=7; upper segment 3×7 horizontally oriented exact-four bodies,
stance columns 4/11/18/25 and rows 2,4,...14. Lower band 1×13 vertically
oriented exact-four bodies, stance rows 20/27 and columns 3,5,...27.
Total 34 bodies/152 shared braces; optional global transpose from
visible proximity to the exact bodies and worker anchors. Do not vary
the offset/line count. All physical sides and stance axes are stored
as actual cells; brace-progress aggregation uses the actual local
orientation. Exact flood proves all-open/all-closed exterior connected.
The root's corresponding prebuilt ideal passive calculation at t120
(C/P/R) was .7248/.8568/.9280 vs baseline .6897/.8186/.8986; this is not
the live staffing or capture metric.

Staging uses the same jobs/stance distances as current rescue, with
local 5 and global 23/34 (ceil of 2/3). t101 assignment has 4 distinct
top guard roles and 2 perpendicular lower ones; M>=8 duplicates both
lower roles for opposite column halves (8 total). M7 uses one assembly
extra; leftover workers only do ordinary brace work (no rover).
Unchanged exact legal two-gate priority, fallback schedule t160 if
observed residual>4 else t225, and unchanged tactical fallback.
Existing confirmed narrow rescue policy is byte-exact when M<7.
No submitted/participant implementation used.

| Used common M>=7 dev30 | Narrow default (straight3×12 in group) | Orthogonal |
|---|---:|---:|
| Exact official v3 score | 575482884 | 498274502 |
| Direct exact-four animals | 201 | 220 |
| Final exact-four animals | 207 | 226 |
| Staffed legal windows | 197 | 215 |
| Unique eligible animal IDs | 384 | 393 |
| Ready bodies t50 / t101 | 688 / 1052 | 660 / 921 |
| Handoff t160 cases | 29 | 28 |
| t160 residual | 271 | 251 |
| Actual handoff residual | 269 | 250 |
| Reachable handoff area sum | 21164 | 21298 |
| Final residual | 62 | 75 |
| Fully clear | 10 | 6 |
| Pet-free humans | 105 | 74 |
| Distinct final safe area | 4059 | 2176 |
| Median/max tester wall sec | .378/.572 | .412/.592 |

W/L 14/16. Direct captures by species cow/pig/rabbit/dog/cat
[36,30,29,62,44] → [42,25,36,68,49]. Captures per bank
[62,55,84] → [48,30,52,90]; perpendicular lower region accounts for
90 genuine exact-four captures. Handoff to terminal residual drop
207 → 175 across paired case cohorts; this is observable decline in
cleanup efficacy despite stronger handoff, not isolated causal proof
about a particular line or wall. All 76 legal official tests and
independent replay matched; all eight M<7 controls whole action and
geometry bytes identical. No held-out or official distributed100 run,
no promotion. Nominal score default remains narrow rescue.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-orthogonal.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-orthogonal.patch"
g++ -std=c++23 /tmp/ahc008-orthogonal.cpp -o /tmp/ahc008-orthogonal
g++ -std=c++23 "$D/parallel-banks-narrow-rescue-research.cpp" -o /tmp/ahc008-narrow
python "$D/parallel_banks_orthogonal_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-narrow /tmp/ahc008-orthogonal "$WIDE_DEV/eligible" /tmp/orthogonal.csv
python "$D/parallel_banks_orthogonal_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-narrow /tmp/ahc008-orthogonal "$WIDE_DEV/low" /tmp/orthogonal-id.csv --low-workers --case-count 8
```
