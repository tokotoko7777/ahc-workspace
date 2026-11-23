# Fixed Map1 top-edge interior pair at aisles c4/c14

Representative stock top edge, not physical corner retirement, same
two original nominal sites (Map1 IDs 1 and 2), not Map3 perimeter.
Editable set only r0..1, c5..13 inclusive. Preserve every other wall,
door, body, and old stance; c4/c14 are physical stock lanes. Existing
four X in edit. Target at most three planned X in this set.
Place one direct horizontal inward-facing solo mouth per side, choose
row0 or row1 independently. Same-row outward guard is on c4 or c14
and may not be inside a room. Require each nominated body to use
actual top row r0, have size 2..4, and two unblocked pet-safe parities.
Search exact edit wall masks and four gate-row pairs, no other moves.
The previous no-touch/corner-retirement shortlist did not redesign
these two interior edge pockets while retaining both roles. The r3
retained stock room has a notch at r2 c9, so its exact standalone
lock forces stock (r1,c9) to remain X; use as static necessary prune.

Require old other stock roles exact under their own lock, nominees
exact in either/both locks, physical all-open and own-lock exterior
and aisle/stance service, no new orphan floor in virtual all-closed
layout. Only the actual board perimeter is free bracing; row2 and
other topology are original full physical terrain. No solver, game
trace, case input, score, alternate patch, or Git action.

Once: PYTHONDONTWRITEBYTECODE=1 timeout -k 1s 10s python3 check.py
