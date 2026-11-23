# Finite mixed strip capacity results

The permanent exterior connector proof bounds total plus paired rooms by
116, below 75+45, for the specified direct-lane roles.
With the permanent connector removed, the frozen C++ column-profile
relaxation was run once at the specified compile/search caps, both exited
0. The exact source digests and every signed table interval are in
`output.txt`.

For at least 45 paired rooms, relaxed whole-board total is bounded by
**71 ≤ relaxed optimum ≤ 75**. Fifteen queries capped: three at central
width 3, eight at central width 4, and one each at widths 5 through 8.
No edge query capped. The exact profile edge caps in widths 2,3,4,5 are
6,10,13,15 rooms. In particular the combination of three 15-paired
width-4-body banks and two 15-solo edge-width-4 banks is excluded.
The optimistic bound still touches 75, so this is **inconclusive**.
No physical packing or ≤223 X guarantee is implied by the profile
lower bound. No game data, benchmark, solver/tester or layout ran.
