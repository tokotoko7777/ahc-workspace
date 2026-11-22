# Truly idle service-lane patrol (old dev30, not adopted)

Try moving only a guard unused by the normal bank controller, beginning
on zero-based turn 101. Ordinary simultaneous closes, pair assignments
(including a guard already waiting in position), scheduled ordinary
movement, construction, geometry, t160 rescue threshold and all fallback
behavior are retained. Patrol only after normal command generation.
From a guard's own current lane and assigned half when split, one step
toward a prepared, open neighboring tube row at distance <=3, oriented
toward an observed reachable pet within distance <=3 of the two central
body cells. Exclude animals currently being pursued by ordinary selected
pairs or already being caught, planned wall cells, and ordinary move
destinations. Deterministic per-pet/per-guard matching avoids duplicate
animal targets. No relocation off the lane or across a half boundary.
This is inspired by the aisle-based human organization in the
[user-authorized TERRY narrative](https://blog.terry-u16.net/entry/ahc008),
not any submission code. No-args keeps rescue. Flags `5 1 1` enable patrol.

| Used dev30 paired official | Rescue | Patrol |
|---|---:|---:|
| Score | 697433161 | 698312298 |
| Direct simultaneous exact-four captures | 199 | 197 |
| Final exact-four capped pen animals | 205 | 202 |
| t160 reachable animals | 246 | 247 |
| Cases released at 160/225 | 24/6 | 24/6 |
| Final residual animals | 40 | 46 |
| Whole-case all-clear | 14 | 10 |
| Cleared people | 113 | 86 |
| Distinct safe area sum | 5026 | 4301 |
| Actual post-release nonrectangle 4–48 captures | 48 | 57 |
| Median/max tester seconds | .366/.464 | .386/.653 |

Score W/T/L 10/13/7; all 60 runs legal and exact on official and configured
independent replay, control matches frozen full action hashes, t0–100
inclusive matches in all 30 pairs. Five cases have exact whole transcripts.
The 218 patrol commands all survived the higher-level legality and
closing guards; 207 turns across 25 cases. No observed reduction in the
bank residual despite departing from idle; the tiny score gain is not
enough given lower all-clear, less clean area and slightly fewer actual
simultaneous tube captures. Later nonrectangular capture counts are real
but include ordinary fallback effects, not demonstrated causal wins.
No promotion, no fresh or official comparison, no main changes.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-idle-patrol.cpp "$D/parallel-banks-rescue-research.cpp" "$D/parallel-banks-idle-patrol.patch"
g++ -std=c++23 /tmp/ahc008-idle-patrol.cpp -o /tmp/ahc008-idle-patrol
python "$D/parallel_banks_idle_patrol_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-idle-patrol "$DEV30" /tmp/idle-patrol.csv --frozen-control-csv "$D/parallel-banks-rescue-dev30.csv"
```
