# Map3 large edge belt, corrected handoff: still rejected

The first variant's off-aisle handoff bug has a fixed-board regression.
Only defer c26 returns for empty-original-queue actors already in c27–29:
the belt places a guard or restores the existing return. The same toy
goes from R,L,R,L to R,R,R,d and then returns. The founding physical
layout, new build window and hunter scoring were not changed.

A completely fresh unfiltered dev100 paired official test covers 40
selected map3 boards and 60 other boards. All 200 real runs passed
official tester, official playback and independent scoring.

| Metric | Accepted short stochastic reference | Corrected belt |
|---|---:|---:|
| Overall score | 6,088,722,221 | 5,462,263,024 (−10.2888%) |
| Full / useful full | 97 / 97 | 81 / 79 |
| Final uncontained | 3 | 36 |
| Affected map3 score | 2,586,444,441 | 1,959,985,244 (−24.2209%) |
| Affected map3 full / useful full | 39 / 39 | 23 / 21 |
| Affected map3 final uncontained | 1 | 34 |
| Max local tester | 2.8872 s | 2.8654 s |

All 40 built all 20 belt walls, with 67 fewer actual planned walls.
New-mouth closure counts across those 40: 0×13, 1×16, 2×10, 3×1,
for 39 mouth locks total; no four-lock case. There were zero new locks
in the separate broken pilot, not a paired fixed-versus-broken score
comparison. Spatial functionality was repaired but the change
could not absorb the factual prior paired-bank load. Whole map3
20 wins and 20 losses, +9.04 mean human floor only on the 23 shared
complete boards; the dual requirement of nondecreasing captures and
+35 common-clear floor fails. Other 60 maps are byte-identical with
same scores. No held/practice runs; accepted best unchanged.

Independently written source transform:
`71c76fba12798b2d7d2b86a9e0963b868eb4f497eb8ce620873b36b3bab03f79`.
Rebuild original accepted local h123 source, first pilot source with
make_live.py, then apply relay_fix.py and compile GNU C++23. Precommitted
seeds, official case/pair deltas and logs in the companion gzip;
first archive contains transforms and fixtures, not third-party input.
