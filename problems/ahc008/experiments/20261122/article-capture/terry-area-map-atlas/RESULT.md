# Existing five-map area ceiling: enough room in hindsight, not a selector

New unfiltered generated100, accepted incoming implementation, the
same fixed zero late-rollout cap (Normal-only) in all five existing
stock layouts. For each case, evaluate the original selector's
initial model choice alongside the others under identical budgets.
All 500 official tester outputs, playback traces and independent
scores match. These are not the standard clock or practice set.

The original selector catches all in 91 of these Normal-only cases.
Only on those cases, choose highest final mean legal worker floor
among maps which are also complete, otherwise do not credit.
32 of the 100 cases have higher full-clear floor, summed +1,193
mean-worker tiles, i.e. **11.93 per tested board**. This passes the
predeclared 1,000/20 investigation threshold. It is a retrospective
upper envelope, not a causal score gain or an actual selection rule.

| Fixed map | Complete of 100 (Normal-only) |
|---|---:|
| 0 | 86 |
| 1 | 94 |
| 2 | 93 |
| 3 | 69 |
| 4 | 87 |

Of 32 favorable cases, 19 are reference map2→best map3, 5 are
map1→2, 4 are map4→3, and 4 other. Blind map2→3 on the 28
reference-map2 cases reduces full catches 28→19; no blanket
replacement. Both paths need independent validation. Outer X
reduction on those known fulls is part of the gap, and layout
choice can trade away useful score if it misses even one head.

Rebuild private no-simulation model selector with make.py and plain
GNU C++23 from the current accepted incoming source, using it only
to label the reference. Regenerate the frozen fresh atlas100 with
prepare.py. run.py uses the unchanged accepted solver and official
paired playback, with each map explicitly given and all late pair
caps zero; these are benchmark controls, not submission cases or
policy. The bundle contains tools, 500 run hashes, all results and
seed manifest, independently authored transforms only. No supplied
selector model or generated binaries are redistributed.
