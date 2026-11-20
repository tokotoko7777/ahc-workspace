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

## Follow-up: two consecutive reflections

Reflecting around the distinct circle tokens c and d consecutively sends
launch x to `x + 2*(d-c)`. The intermediate landing and final landing
both constrain x, so the legal interval is intersected with both
constraints before clipping the same 1D minimizer. Both consumed
tokens are replaced by the respective pre-reflection positions, even
when multiple tokens have the same coordinate. Keep the frozen
one-jump result in every variant. Both the original larger-budget
width-64 single-jump search and an independent workload-matched
width-28 control are recorded rather than treating the former as fair.

| Fresh set, 28 cases each | Frozen ops ↓ | Single-jump control ↓ | Two-jump ops ↓ | vs control |
|---|---:|---:|---:|---:|
| Larger control dev | 11358.679 | 10899.821 | 10980.714 | 10 / 5 / 13 |
| Larger control holdout | 11315.571 | 10909.214 | 11180.286 | 8 / 4 / 16 |
| Matched dev | 11531.036 | 11265.321 | 11116.071 | 11 / 7 / 10 |
| Matched holdout | 11213.857 | 11172.000 | 10966.607 | 13 / 7 / 8 |

In matched holdout, extra probes were 7924 vs 7350 and median wall
.00417 vs .00421 s. Median paired advantage vs single jump was zero;
mean improved by 205 operations. Keep the alternatives opt-in. Both
fresh sets of 56 cases (336 full outputs) agree with exact independent
replay and the official visualizer. Max wall .015458 s. Checked the
two-reflection axis range exhaustively in the library test.
[Large-budget log](benchmark-double-wide-results.csv) ·
[Matched log](benchmark-double-matched-results.csv).

Next explore each saved circle's eventual use beyond the next immediate
goal. The today-cheapest launch rectangle can make poor future racks;
include a small explicit offset with its exact launch cost, then compare
against a wider exact matched-work control on new data.

```sh
p=problems/ahc073
d=$(mktemp -d)
for i in 0 1 2 3; do
  g++ -std=c++23 -O3 -DAHC073_DOUBLE=$i "$p/main.cpp" -o "$d/arm$i"
done
python "$p/benchmark_double.py" /path/to/gen /path/to/vis \
  "$d/arm0" "$d/arm1" "$d/arm2" "$d/wide.csv"
python "$p/benchmark_double.py" /path/to/gen /path/to/vis \
  "$d/arm0" "$d/arm3" "$d/arm2" "$d/matched.csv" --seed-salt double-reflection-matched-v1
```
