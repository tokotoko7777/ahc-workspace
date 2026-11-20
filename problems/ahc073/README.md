# AHC073 — Circles of Inversion

[Canonical official statement](https://atcoder.jp/contests/ahc073/tasks/ahc073_a) ·
[Official generator and visualizer](https://img.atcoder.jp/ahc073/A21qfZSs.zip) ·
[Official ranked result](https://atcoder.jp/contests/ahc073/results/json).

For a complete itinerary, score is `M + T − operations + 1`, higher
better. Incomplete itineraries get `visited + 1`. Sum over the 150
original contest cases; there is no system rerun. Official ranked metadata
names **hitonanode** first, without numeric score. Both direct official
case/score archive paths return 403 and the rank-only ALE archive path
returns 404. Historical numeric target and exact case identities are
unverified, so strict historical comparison is **unjudged**.

## Exact one-jump leg geometry, with state search

Direct Manhattan walking is a legal fallback. From current waypoint
`a` to next waypoint `b` and circle center `c`, choose a launch `x`
to minimize `|a−x| + |(2c−x)−b|` per axis. The interval between `a`
and `2c−b` gives all unconstrained minimizers; clip both endpoints to
the feasible launch interval where both launch and landing are inside
the board. The product is an exact rectangle of equally short launches.
Their costs agree but the replacement circle's future value differs.
We sample its corners, center, and one point closest to the following
leg's ideal center. Keep no-jump as an option. Reaching future goals
prematurely does not skip their eventual ordered visit.

Both arms share a first width-one path using an initial rack with
three early-leg midpoints. The control additionally tries 24 greedy
routes with jittered early midpoint racks; the variant instead uses
one width-24 beam with the same base rack. At each waypoint, canonicalize
and deduplicate the circle multiset, retain actual cumulative cost for
each state, and give a one-leg future saving a small priority bonus.
Widths/trials target comparable work; actual candidate probes are
logged. Every selected route is fully reconstructed with at most T
operations.

| Fresh set | Greedy portfolio ops ↓ | Circle-state beam ops ↓ | Beam win / tie / loss |
|---|---:|---:|---:|
| Dev (28) | 11,194.179 | 10,889.893 | 19 / 0 / 9 |
| Holdout (28) | 11,592.536 | 11,155.000 | 22 / 1 / 5 |

Holdout mean official score 1,384.00→1,821.54; median save 340.5
operations. Mean candidate probes 6,735 control / 7,664 beam, median
walls .00278 / .00288 s. All 112 routes complete and independently
match the official visualizer, with exact counter for colocated circles,
both boundaries, operation cap and ordered targets. Max wall .005064 s.
Beam is branch default. [Full paired logs](benchmark-results.csv).

No existing helper returned all equal-cost launch choices under reflection
boundaries. Added [`reflection_launch_range`](../../library/reflection-launch-range.hpp)
and checked all launch coordinates on 36,100 small cases.

**Next hypothesis:** allow two consecutive reflections (with no intervening
walk) using distinct circle tokens. The composition is a translation, so
the two-reflection cost again has a clipped minimizing rectangle. Compare
the extra state transitions against a wider one-jump beam at equal
expansion work. Independently validate both reflected positions and
the multiset update, including colocated circle tokens.

```sh
p=problems/ahc073
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC073_BEAM=0 "$p/main.cpp" -o "$d/greedy"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/beam"
python "$p/benchmark.py" /path/to/gen /path/to/vis "$d/greedy" "$d/beam" "$d/results.csv"
g++ -std=c++23 -O3 tests/reflection_launch_range_test.cpp -o "$d/test"
"$d/test"
```
