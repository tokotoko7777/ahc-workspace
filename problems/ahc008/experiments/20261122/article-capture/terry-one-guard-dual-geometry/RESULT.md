# One guard closes two mouths on consecutive turns: geometry screen

Question: Can a one-to-four-cell pocket replace a two-worker capture
with one worker who shuts both mouths from one fixed exterior cell?
More mouths are useful only if the first closure leaves a robust
opportunity on the second turn. This is a static obstruction screen,
not a probability estimate or a new AHC008 result.

Enumerate the fixed polyominoes of areas 1 to 4 up to translation only.
All body-boundary cells except two nominated gates are solid. Both gates
touch the connected exterior (far side of those two cells when sealed),
and one exterior cell is adjacent to both. The only animal starts in
the body; no outside animal can block or affect the operation. Try both
orders and every initial position. The first gate must be legal; all
possible official uniform neighbor step sequences (one for cow, two
for pig, three for rabbit) must end in the body and outside the
remaining gate's forbidden adjacent cells. The second gate can then
close from the same stance.

| Body cells | Oriented shapes | Physical two-gate choices | Robust cow choices | Robust pig choices | Pig choices safe on both starting colors | Robust rabbit choices |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 1 | 4 | 0 | 0 | 0 | 0 |
| 2 | 2 | 8 | 0 | 0 | 0 | 0 |
| 3 | 6 | 28 | 16 | 0 | 0 | 0 |
| 4 | 19 | 100 | 92 | 56 | 0 | 0 |

Each of the 56 pig choices has exactly one safe starting cell, taking
the union of the two first-gate orders. Both gates have the same
checkerboard color because they share a neighboring worker cell.
A pig running two steps preserves its starting color. A robust
remaining gate needs distance at least four at launch; for a body of
at most four cells this pins that starting color. At distances 1 or 2
one can end on the gate (or beyond), and at distance 3 one can end
adjacent. A rabbit can reach a distance-four gate's adjacency in three
steps as well. The enumerator independently uses all move sequences,
not a distance cutoff.

No full-board tiling, worker timing, multiple pets, dogs or cats,
probabilistic closes, or moving guard after the first turn are tested.
The count uses interior pockets in an otherwise empty grid, not
board-edge-cap variants. These are no grounds to safely replace the
general paired rooms, though selectively supplementing them for one
pig starting color or cows is not ruled out. No solver or official
tests were run. The current accepted source and score are unchanged.

Reproduce with `python3 audit.py > /tmp/ahc008-one-guard.json && diff -u result.json /tmp/ahc008-one-guard.json`.
