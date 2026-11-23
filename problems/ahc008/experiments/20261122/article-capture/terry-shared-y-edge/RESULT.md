# Shared Y closure vs two ordinary edge mouths

Fixed 5×9 four-neighbor patch. Only top row r0 is a physical board edge.
Bodies cannot touch other patch boundaries; those boundaries remain
real floor/brace/door/stance tiles. Count required planned X exactly
and separately from any eventual closing walls. No game data.

## Exact enumeration reproduction

Enumerate every connected body mask of sizes three and four in rows 0–3,
columns 1–7 (start with singleton masks; add any in-domain adjacent
pixel and deduplicate after each size). For each body B, consider each
physical tile g in its in-patch orthogonal outer boundary. Put required
walls exactly on boundary(B) minus {g}; keep only if the body tiles not
adjacent to g include both checkerboard colors and at least one
neighbor of g is outside the body, gate and walls. There are 447 and
1,706 one-mouth body/gate templates of sizes three and four.

For each ordered template pair of the chosen sizes, reject any body
overlap, adjacency between bodies, or body/door conflicts with the
other shell. Gate overlap is allowed solely for the Y arm (g1=g2);
the ordinary arm requires distinct gates. Union required walls and
exclude both bodies and both gate tiles. The remaining exterior must
be nonempty and one connected component. For the ordinary arm each
gate must touch exterior (these include distinct stance witnesses
below). For the Y arm the shared gate must touch exterior so a single
actor can close it. With both gates open all floor must be connected;
boundary masks prove respective bodies exactly isolated when their
doors close, while exterior stances do not strand. Minimize the
union wall count per size/mouth case. No optional walls, no clipped
side brace outside the tested patch, no physical bottom-edge waiver.

| Body sizes | Separate-mouth planned X | Shared-Y planned X |
|---|---:|---:|
| 3 + 3 | 6 | 7 |
| 3 + 4 | 7 | 8 |
| 4 + 4 | 7 | 8 |

## Exact 3+3 minimizing witnesses

Both use bodies {(0,1),(0,2),(1,2)} and {(0,4),(0,5),(1,4)}.
Coordinates are zero-based.

* Y: one mouth (1,3), one guard stance (2,3). Walls
  {(0,0),(0,3),(0,6),(1,1),(1,5),(2,2),(2,4)}.
* Separate: mouths (0,0), (0,6), respective stances (1,0), (0,7).
  Walls {(0,3),(1,1),(1,3),(1,5),(2,2),(2,4)}.

A straight 1×3 with one endpoint gate requires seven braces in the
interior (two long sides plus one cap) and four when parallel to the
physical edge, before sharing walls with another room.

## Area and role implication

If both pockets actually need to close, +1 planned X offsets the
shared gate's one fewer closing wall, with the same total isolated
body area. If the distinct-gate layout needs only one 3-cell pocket,
its other pocket stays open as human floor, while the Y necessarily
isolates both. For a 3+3 pair the Y therefore loses four retained
tiles (one additional planned X plus three unnecessarily isolated
floor cells), conditional on equal other connectivity and no leaks.
The single closing action can use one actor on that turn, not two;
this is a staffing option only. Closing irrevocably abandons a
currently empty side. Actual simultaneous occupancy, capture
acceptance, and throughput were not measured. This is neither a
30×30 lower bound nor a live score conclusion.
