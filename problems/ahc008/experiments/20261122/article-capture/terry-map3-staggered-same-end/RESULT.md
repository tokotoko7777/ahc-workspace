# Map3 staggered seam: same-end two-mouth room

Static verified on the fixed staggered-left Map3, using
../ahc008_terry_map3_staggered_left/result.json as the input geometry.
Coordinates zero-based (row,col). Room 40 retains body
{(17,2),(17,3),(17,4)}. The two alternatives are independent:

| Side | Retained gate | Former brace now second gate | Former opposite gate now X | Simultaneous stances | Recessed exit | Both legal body parities |
|---|---|---|---|---|---|---|
| West | (17,1) | (16,2) | (17,5) | (17,0), (16,1) | (16,1)→(16,0) | (17,3), (17,4) |
| East | (17,5) | (16,4) | (17,1) | (17,6), (16,5) | (16,5)→(16,6) | (17,2), (17,3) |

Reproduce exactly, without changing any other map masks: overlay each
row in the table separately, removing the specified old brace from X
and adding the old opposite gate to X; replace just room 40's semantic
door/stance sets. On the four-neighbor 30×30 grid, flood from (0,0)
with only planned X blocked; expect all 667 open floors. For each
of the 75 rooms individually add its doors as walls and verify that
the body flood equals the nominal body and the remaining flood from
(0,0) includes every other open cell and all aisle/guard stances.
Then block all current doors and verify each of the 75 body floods
again. Compute room 40 safe endpoints by excluding any floor
cardinal-adjacent to either of its doors. Test the direct recessed
exit under its own lock. Each alternative retains 233 X and the
exact other 74 roles/footprints, while removing one true through route.

The doubled separator leaves r16 isolated from the r14-notch chamber
by the still-required r15 shell. The usual single separator rows sit
beside another unchanged aligned chamber: opening an end brace there
would enlarge that neighbor when it alone closes. Room r14 was not
part of this strict three-cell mechanism: its adjacent new door
touches both end tile and axial notch. The recessed stance touches
both gates and therefore can serve eventual sequential research,
but this note says nothing about movement robustness or animals.
The checked roles here support simultaneous legal placements conditional
on the listed safe positions and no other placement blockers.
No traffic, solver, or performance tests.
