# Direct-lane strip profile: tighten upper bound to 74

The exact output of the earlier frozen profile relaxation is
`output.txt`, SHA-256
`490feecdec484519f67cd7c2eaab91b058245c24d6ade6d086a06e2bc21e52e9`.
This followup only recombines that existing table, not the profile
search, a solver, a physical layout, or any game data. The earlier
archive is unchanged.

From this folder, run `python3 recombine_readonly.py`. The memoized
recurrence takes the remaining charged width, remaining paired count
(clipped at zero), and first allowed interior row-table index. At each
step it either appends an interior row option at a nondecreasing index
or, once 45 paired are reached, finishes with an unordered pair of
physical edges. A terminal pair may leave unoccupied columns. Thus
every unordered multiset in the original optimistic table occurs
without relying on order and all maximizers are retained. The only
upper-75 combination is:

| Quantity | Central width | Paired each | Solo each (optimistic) | Charged width each |
|---:|---:|---:|---:|---:|
| 3 | 4 | 15 | 0 | 7 |
| 1 | 5 | 0 | 30 | 8 |

No edge banks. Charged width is 29 plus one initial guard aisle.

The width-five central solo strip cannot pack 30 rooms even in the
profile relaxation. A room facing either side uses that first body
column, and its touch incurs a +1 charge. The 31 slots per column
allow at most 15 rooms per side (one body+one charge each), so a
30-room bank requires 15 facing each side. Each such side column has
only 16 possible body cells left after the 15 touch charges. A single
span-one body would take four cells, others at least 14, contradiction.
Therefore all 30 rooms span at least two body columns, requiring a
total of at least 30×(4+2)=180 slots, more than 5×31=155. So the
optimistic maximizer is impossible, and the relaxed global upper is
**at most 74**. The previous lower 71 refers only to the profile
relaxation; no wall count, floor score or physical layout is established.

## Conceptual local witness target (not built or verified)

For recessed roles, consider a 7×9 module with exterior side aisles
columns 0 and 8. Put 1×4 bodies at (row2, columns2–5) and
(row4, columns3–6). Gates are row2 col1/6 and row4 col2/7, with their
outward stances row2 col0/7 and row4 col1/8 respectively; each recessed
stance is just one step from its permanent side aisle. Brace rows 1,
3, 5 cover columns 2–5, 2–6, 3–6 respectively; rows 0/6 open. This
breaks the direct-strip exclusion: a gate column for one room is a
body column for its neighbor. Start with this as the 13-X control,
then seek a two-room stagger that keeps both exact through roles with
at most 12 X; require physical isolation under each single lock and
no worker/outer-port disconnect. A 12-X candidate would demonstrate
recession without an immediate shell tax; it would not certify
whole-board capacity. No code/fixture search is proposed in this note.

A straight 1×3 with two opposite endpoint mouths has only its center
safe from both gate adjacencies, so cannot supply both pig parities.
With one endpoint mouth and a solid far cap, its center and far
endpoint remain safe and have opposite parities. This narrower solo
option changes the exact-four design, not just its guard role.
