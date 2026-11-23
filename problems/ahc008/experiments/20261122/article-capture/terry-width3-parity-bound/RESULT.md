# Necessary parity-capacity bound for width-three opposing paired rooms

Use a rectangular H×3 physical body slab. Every named room body is
connected, lies wholly in the slab, and has opposing axial mouths just
outside its left and right sides. Each mouth touches its own body.
Distinct bodies cannot be cardinal-adjacent: then their specified
footprints could not be isolated independently, even when all mouths
are blocked. No assumption is made about reachable paths in the exterior.

Every one of the k bodies contains a tile in each of the three columns,
since a connected path from its left neighbor to its right neighbor
must cross the middle column. In any fixed column, scan all occupied
vertical runs. By non-adjacency, every run carries only one body label.
All k labels appear, so there are at least k runs, even if some labels
appear in multiple runs. Consecutive runs require a non-body gap. Thus
at least k−1 of that column's H cells are non-body and the three
columns supply at most 3(H−k+1) total body cells.

A three-tile connected body spanning three columns must have exactly
one tile per column and both path edges horizontal. Its two end tiles
are adjacent to the respective axial mouths and cannot be the safe
endpoint of a legal two-mouth closure; only its center remains, with
one checkerboard color. A body safe on both pig colors needs at least
four tiles. If s bodies have this property, total body area is at
least 3(k−s)+4s=3k+s. Consequently every feasible layout satisfies

    s <= min(k, 3(H−2k+1)).

If the right side would be negative, even the proposed k bodies
cannot coexist. For H=30, the bounds are s≤3 at k=15 and s≤9 at
k=14. If s=k is required, then

    7k <= 3(H+1).

At H=30 this gives k≤13. These are necessary, not constructive:
wall budget, safe gate patterns, stance/exterior connectivity, named
neighbors and traffic can impose further limits.

## Fixed Map3 three-row patch

Take the rows 9,10,11 and body columns 2,3,4, with two opposing
paired rooms. Each column must contain both labels without vertical
adjacency. In its three positions the only possibility is one body
tile at row9 and the other at row11, with a non-body gap at row10.
Thus each body has exactly three cells and neither has both pig-safe
colors. This does not depend on the ≤3 planned-wall bound or on
checks of existing exterior and neighbor roles.

This bound is only for intact axial opposing sides and wholly
contained independently isolatable bodies. It does not rule out
solos, nonaxial/same-end doors, shared closure, or wider slabs.
No code, geometry search, live test, or Git action.
