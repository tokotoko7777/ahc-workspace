# Rejected: map3 last-three-column edge belt

This follow-up was frozen before reading the corrected large-bay pilot.
It changes only pruning cutoff from c>=20 to c>=27, preserving the
full original paired bank c20/c25. Same relay fix, four new vertical
one-mouth four-cell traps, actor selection and timing. It retains
60 original rooms/doors exactly, retires 15 old edge solos, saves 9
planned X (233→224). Static stance, open/closed components, routes and
wall construction validate. Deterministic no-pet staffing passes
40/40 whole belt by t133 with no retained stock-wall losses.

| Fresh generated paired 100 | Accepted | Right edge only |
|---|---:|---:|
| Overall score | 6,006,629,632 | 5,915,240,741 (−1.52147%) |
| Overall full / useful full | 95 / 95 | 92 / 92 |
| Overall remaining animals | 6 | 9 |
| Affected map3 score | 2,280,111,111 | 2,188,722,220 (−4.008%) |
| Affected map3 full / useful full | 35 / 35 | 32 / 32 |
| Affected map3 remaining animals | 0 | 3 |
| Max local tester | 2.75840 s | 2.80594 s |

All 200 real runs match official testing, official playback and
independent replay/scoring. All 35 map3 build every new planned cell,
saving 9 actual intended-wall tiles each. Mean retained human floor
is +2.5 among the 32 common full map3 cases (<+3 required). New mouths
closed 0×20, 1×11 and 2×4. There are 25 wins and 10 losses on map3;
the 65 untouched cases are byte-identical and score-identical. The
three raw/useful drops outweigh the structural savings. Reject; do
not run held or official practice. No adopted change or rank claim.

C++23 candidate source SHA-256
`cd5207fb3f834fcbc67f51a28d2e15635874e522b6d9017ae4c1dd355e822171`.
Rebuild from corrected wide belt with make.py, then compile live and
dry units; use prepare.py dev100 and the same sequential paired
official tester, and map3-belt analyze.py for case/capture deltas.
The companion archive contains independent transforms, tests, seeds,
input hashes and all results, not the supplied selector data.
