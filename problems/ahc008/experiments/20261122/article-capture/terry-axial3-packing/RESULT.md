# Exact geometry profile for a width-three through-room bank

Thirteen connected body/gate templates (eight supporting both safe
pig colors). Enumerate all normalized connected 3/4-cell bodies
spanning three columns with opposed axial gates. Each has height at
most two. Distinct exact rooms cannot be cardinal neighbors. In each
column there must be at least one tile from each of k rooms and at
least k−1 gaps between labels; the total is at most 3(H−k+1) body
tiles. A through three-cell body is necessarily straight, so one
safe color only. Each both-color body costs at least one extra tile
over 3k, so at height 30, at most 3 of 15 and 9 of 14 can attain it;
at most 13 rooms can be both-color.

The independent finite template DP also witnesses the geometry
limits. Adjacent template min-row coordinates differ by at least
two (confirmed by exhaustively checking all template pairs), so
nonconsecutive rooms cannot interact. The height calculation checks
pairwise body/door/shell compatibility and minimizes last origin
per count and template. For minimum X, retain the absolute last
pose and add this clipped shell's size minus overlap with the
preceding shell. Nonconsecutive shells do not overlap: each spans
normalized rows −1..2 and nonconsecutive origins differ by ≥4.

| Total rooms | Maximum both-color at height 30 | Minimum planned X with that many both-color | Maximum both-color with stock budget 44 planned X | Minimum X at that budget-leading count |
|---:|---:|---:|---:|---:|
| 15 | 3 | 44 | 3 | 44 |
| 14 | 9 | 45 | 7 | 44 |
| 13 | 13 | 45 | 11 | 44 |
| 12 | 12 | 43 | 12 | 43 |

All four budget-leading witnesses independently flood the entire
two-rail patch when open, and under each own-room closure the
exact room isolates while all other free floor stays in the two-rail
exterior. Both true board ends clip shells; side shells on the
gate columns count as X. This is a standalone strip, not a result
for unchanged surrounding roles, construction, other pets,
traffic or score. Retaining 15 slots cannot broadly fix parity
by relocating the separators in this physical width. Dropping a
slot is a possible tradeoff, not an assumed adoption.

Reproduce: `python3 check.py > result.json`, then
`python3 minwalls.py > walls.json`. No game/contest input used.
