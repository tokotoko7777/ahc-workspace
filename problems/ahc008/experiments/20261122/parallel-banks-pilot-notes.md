# Live parallel exact-four tube banks

Experimental implementation of the user's repeated parallel-tube
layout, not a single edge rail. Choose orientation and one lane offset
from visible initial positions. Four continuous aisles have spacing
seven. Between adjacent aisles, 12 two-ended tubes each provide four
interior floor cells, with an independently closable gate at both ends.
The 13 brace segments per bank are length four (over the interior only,
not the mouths). This allows vertical gate exits before closure and
makes those neighbors relevant to actual build legality. Both crossbars
stay open. Total 36 tubes and 156 initial brace jobs, and new closures
cost exactly two walls per actual four-cell component.

Until t101 all workers may complete nearby brace segments, favoring
already-started segments, and take already-staffed legal closures.
Assign one guard to each aisle thereafter with minimum summed travel.
Guards weigh useful targets on a common neighboring row and match outer
banks vs middle; extras build until t133. All humans stay outside a
sealing tube, and all official occupancy/adjacency rules apply. At t225
return to existing tactical fallback on the actual built board. This
pilot makes no claim that this final fallback is well suited to the maze.

On the original generated dev30, all 60 official/replay outputs legal.
Bank actions sealed 127 exact-four components at real occupied gate
windows in 26/30 cases (101 repeats after the first in any case), with
133 distinct directly captured pets: cow/pig/rabbit/dog/cat 21/24/17/46/25.
Same guard calculation also found 128 actually staffed legal windows
(the final action filter declined one). Noneligible primary causes:
1,480 window snapshots lacked built braces and 2,711 were physically
legal but had no two staff at the mouth stances; repeated turns are not
independent opportunities. There were 7,338 occupied but gate-blocked
turns including nearby pets along open mouth-side exits. By t101
median 156/156 brace cells and 36/36 tubes ready; mean per-case median
readiness 43.68. By t225 averages 155.27 brace cells and 35.63 tubes
prepared. Median among the 26 capturing cases, first seal turn 133.5. Guard travel and animal crossing are stronger
next explanations than lack of build completion after t101.

| Metric over dev30 | Unchanged tactical | Bank pilot |
|---|---:|---:|
| Official score sum | 603,294,259 | 69,288,715 |
| Final pets in actual exactly-four components | 4 | 136 |
| Final unique reachable pets | 165 | 265 |
| All-clear cases | 4 | 1 |
| Mean human union area | 371.0 | 644.2 |
| Mean of casewise per-person areas | 263.9 | 471.1 |
| Maximum tester seconds | .510 | .558 |

Four score wins / 26 losses paired. This is a productive explicit
compact-capture architecture across five species, not a score breakthrough.
It spends early jobs on different structures; the watched stage and
unchanged late tactical fallback leave too many residual animals in the
large human component. No promotion.

Compile unmodified tactical separately, then apply
parallel-banks-pilot.patch to another scratch copy, GNU C++23. Pass CLI
mode `1` to the pilot, or use:

    python parallel_banks_bench.py "$HARNESS" "$TESTER" ./tactical ./banks "$CASES" compare.csv

Use the already archived capture-dev30 seed list and official v3 generator
and tester. The wrapper checks actual physical area after every credited
paired closure, official score agreement and exact action replay.
