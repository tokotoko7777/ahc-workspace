# Single residual pet, short irregular finish (dev30; not adopted)

Control: immutable no-argument rescue160 research standalone, tree edition
5b2cda366a4bd0bb7df28d51321636d65563cb59. Trial: unchanged bank and rescue
phases, then at t>=185 **and after the existing release**, only one pet in the
current single component containing every human. Never change the ordinary
action if it already closes the last pet while keeping everyone together.
Start on a third-turn scan no later than t287; at most one planned attempt
per case, 10-turn expiry, distinct outside builders with observed-board
walking ETA <=7, no preemption of ordinary wall commands. Abort if the target
leaves the planned pen or current routes/partition stop validating. Commit all
still missing legal walls atomically, never to an empty pen. The full-band
human service lane columns are immune from overlay cuts. Require true closure
size 4–48, at most six missing vertices, everyone in the same outside
component and more than half the ordinary projected retained area. Source
component seeds are connected four-cell shapes around the actual animal
(including 2×2 when possible); each vertex min-cut uses existing bank braces.
Nonrectangular geometries receive a soft tiebreak preference, not a mandate.

The problem-specific hypotheses are motivated by the two user-authorized
narratives only: [TERRY](https://blog.terry-u16.net/entry/ahc008) describes
late vertex cut and actual builders; [simanman](https://simanman.hatenablog.com/entry/2022/02/28/074838)
mentions species-aware and irregular fences. No participant submission code,
no submit/judge use. All case IDs below are diagnostics, not rules used online.

| Used dev30, official v3 + independent exact replay | rescue | trial |
|---|---:|---:|
| Official score sum | 697433161 | 697544272 |
| Cases all animals separated from all humans | 14 | 14 |
| Final residual animal count | 40 | 40 |
| Summed distinct human-safe union | 5026 | 5027 |
| Actual bank atomic exact-four captures | 199 | 199 |
| Valid official/replay results | 30 | 30 |
| Maximum tester wall (seconds) | 0.4540 | 0.4383 |

W/T/L 1/29/0. Identical action bytes through the start guard t>=max(185,release),
and full traces identical for cases without a plan. Exactly one animal in an
all-human connected component occurred in 15/30 per arm (first occurrences
2 cows, 6 pigs, 7 cats; 10 of these cases eventually all clear in either arm).
Across 106 scans there were 11 feasible seeded cut/crew combinations,
concentrated in four cases. There were **zero overlay atomic closures**.
All four attempted plans changed walking. At 0000 the 7-cell plan failed
revalidation when the ordinary actions changed on the next turn. The cat
(0006) and pig (0011) left the proposed 4- and 8-cell pens respectively
within two and one turns. At 0026 ordinary fallback, not overlay, completed
capture of the last cow in a 4-cell final pen on t281. The action differences
preceding it coincided with one extra human-safe cell and +111111 score;
that case does not establish an overlay capture or general gain. No new
held-out sets were touched. This mechanism has insufficient evidence for
promotion. Closure timing, not merely trigger rarity, remains the observed
bottleneck. This does not rule out a different seed shape or preserving
ordinary fallback work while moving an already-available adjacent worker.

Reproduce from the repository root, with the same official tools v3 and
used dev30 inputs as the rescue freeze. The harness must provide
benchmark.py and benchmark_cases.py. The experiment retains rescue as the
no-argument behavior; flags 5 1 1 opt into the overlay.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-last.cpp "$D/parallel-banks-rescue-research.cpp" "$D/parallel-banks-last.patch"
g++ -std=c++23 "$D/parallel-banks-rescue-research.cpp" -o /tmp/ahc008-rescue
g++ -std=c++23 /tmp/ahc008-last.cpp -o /tmp/ahc008-last
python "$D/parallel_banks_last_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-rescue /tmp/ahc008-last "$DEV30" /tmp/ahc008-last.csv
```
