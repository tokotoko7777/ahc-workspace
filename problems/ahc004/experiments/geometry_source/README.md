# AHC004: Geometry seed, standalone GNU C++23 configuration

`submission_entry.cpp` combines the geometry initializer from measured
`f5d8fed280d6ad1787d9f97fba947c674e36b3ec` with the exact placement/coverage
hot path from `a1eb69c3f2e92a43ec6f867c7225a5826d0e7fe9`. It does not enable
the optional whole-line post-pass or the earlier scouting arm. Geometry only
replaces the ordinary initial board when its completed board strictly wins
on exact duplicate-weighted coverage (then number of empty cells).
The geometry feature gate remains 28 or more recovered cycles and at least
75% supported reads. Its stage checks and whole-search wall target remain
1,000 ms and 2,780 ms, respectively; a started check may run past a stage
boundary. All tiers use synthetic geometry only, never input/seed IDs.

Run the one-file build without external optimization flags; the effective
`#pragma GCC optimize("O3")` is in the generated source.

```sh
python3 scripts/bundle.py problems/ahc004/experiments/geometry_source/submission_entry.cpp \
  -o /tmp/ahc004-geometry.single.cpp
g++ -std=c++23 /tmp/ahc004-geometry.single.cpp -o /tmp/ahc004-geometry
g++ -std=c++23 tests/ahc004_anchor_congruence_test.cpp -o /tmp/ahc004-anchor-test
/tmp/ahc004-anchor-test 128 noisy
```

The prior `f5d8fed` build, evaluated under external `-O3`, obtained
7,505,150,960 on the official downloadable local 0–99 set. This score is
not inherited by the combined source-only configuration: templated helpers
and cycle/anneal wall budgets may behave differently without an external
flag. Retest all 100 local inputs and separate generated cases before
making a quality or timing claim. This integration does not change anything
about comparability with actual contest inputs.
