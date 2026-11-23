# Mixed finite-strip capacity obstruction with permanent exterior connector

30×30 board, exact area-four rooms of any connected shape. Vertical guard
aisles run the whole board. Every horizontal mouth is directly next to its
aisle, with its body at the next column. Between consecutive aisles the strip
has one mouth column on each side and `s` central body columns. A two-mouth
room spans all s of those columns. Solos may face either adjacent aisle and
may share braces with each other or with paired rooms. At a physical edge
there is only one mouth column and no artificial outward boundary cost.

If each body column contains at least one permanently exterior connector
cell, distinct rooms touching that column need at least one vertical brace
each: on either side of that cell, charge each room toward the connector.
Different rooms are separated by a blocked cell whenever their body
intervals lie in the same column, because no gate lies in a body column.
Using one interval per distinct room yields the bound even with
discontiguous intervals of a single room. At least one exterior row slot
is unavailable to body and brace, so `sum(body cells + room incidences)
<=29` per body column. The chosen connector cells need not share a row.
The board-edge row(s) can stand in for the away-side brace, already
accounted for by choosing the connector-side charge.

For an interior strip, P paired and S=L+R solos imply
`(4+s)P + 5S <= 29s`. If s>=2, each of the two side body columns sees
P plus solos facing its own side, each with ≥1 body cell and brace, so
`P+L<=14` and `P+R<=14`. When s=1 all rooms share one 29-slot column.

| central body width s | upper bound on 2P+S | charged width s+3 |
|---:|---:|---:|
| 1 | 10 | 4 |
| 2 | 19 | 5 |
| 3 | 24 | 6 |
| 4 | 28 | 7 |
| ≥5 | 28 (P=0) | ≥8 |

An edge strip of width w=s+1 uses one mouth column. For w=2 area 4 gives
≤5 solos; w=3 aggregate brace/body charge gives ≤11; w≥4 common first
body column gives ≤14. Always solos≤4w. Empty unclaimed edge columns only
consume budget. A single initial aisle plus charged widths accounts for
at most 30 columns, hence `total rooms + paired rooms <=4*29=116`.
A target of ≥75 total and ≥45 paired needs ≥120. No wall budget is needed
to exclude this family. This is not a theorem for recessed stances,
gates in central columns, narrower rooms, or cases without permanent
exterior in every body column. No runtime experiments.
