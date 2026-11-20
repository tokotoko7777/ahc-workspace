# AHC039: Purse Seine Fishing

[Official problem](https://atcoder.jp/contests/ahc039/tasks/ahc039_a) /
[official generator and scorer](https://img.atcoder.jp/ahc039/KNtTkgAy.zip).
2 s, 1024 MiB; one simple axis-aligned polygon with 4..1000 distinct
vertices, integer coordinates in 0..100000, perimeter ≤400000.
Boundary fish count as inside. Absolute score per case is
`max(0, mackerelCount - sardineCount + 1)`. Sum 150 cases; no rerun.

Final #1 **566,997**, terry_u16 by
[official results](https://atcoder.jp/contests/ahc039/results/json).
The direct official raw/metadata CSV endpoints returned HTTP403; the score
comes from the official rank/score CSV in the
[ALE-Bench archive](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc039.zip),
SHA-256 `34d478f166334d6c8a0f7e0d03f6b5c17c557eeff9f163d5510ecc8605a9f3cd`
matching its LFS pointer. The official tools' 100 starter inputs are
not proven identical to the 150 actual cases. **Official comparison is
undetermined.**

## Grid rectangle control vs overlapping strips

Partition each axis near 40 equal intervals, shifting internal cuts
to coordinates with no fish. Closed outer cuts at 0 or 100000 are
kept inclusive. The resulting signed cell counts are exact for outputs
using those cut lines. The control finds the best grid rectangle.

The variant uses contiguous columns with a different vertical interval
in each. Every interval spans a common grid row. Consecutive strips
therefore overlap with strictly positive length and yield a single simple
orthogonal polygon. Try each anchor row and both axis orientations.
At each column, track all possible (lower,upper) states; transition
perimeter contribution is the sum of their endpoint movements.
Track raw count, exact perimeter and predecessor, and only reconstruct
legal results, keeping the rectangle fallback.

Perimeter is handled by several penalties rather than an exact
cost-indexed frontier. The [nonuniform Manhattan max-transform](../../library/manhattan-max-transform.hpp)
applies all previous interval penalties at once in linear time in the
number of states, instead of checking every pair of intervals. It carries
origin IDs for path reconstruction and requires no geometry assumptions
beyond each axis having sorted coordinates.

| 24 fresh cases each | Rectangle mean | Strips mean | W/T/L | Strips median gain |
|---|---:|---:|---:|---:|
| Development | 1,897.75 | 3,342.96 | 24/0/0 | 1,448 |
| Untouched holdout | 1,913.29 | 3,429.71 | 24/0/0 | 1,465 |

All 96 outputs matched official and independent legality and closed
polygon scoring. The longest measured wall time was 0.121809 s under a
1.45 s guard; largest polygon was 64 vertices. The branch defaults to
strips. [Paired results](benchmark-results.csv) do not use final cases.

Next remove the common-row constraint while still requiring positive
overlap of every adjacent interval. Compare exact restricted transitions
(or accelerate their appropriate range maxima) on a fresh split. This
allows the net to follow clusters at different heights without a global
spine while remaining a single legal contour. Separately consider finer
grid boundaries only after the larger shape test.

## Reproduction

From repository root (`ATCODER_TOOLS` points to official tools'
`target/release`, built via `cargo build --release --locked`):

    g++ -std=c++23 -O3 -DAHC039_STRIPS=0 problems/ahc039/main.cpp -o /tmp/ahc039-rect
    g++ -std=c++23 -O3 problems/ahc039/main.cpp -o /tmp/ahc039-strips
    python problems/ahc039/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc039-rect /tmp/ahc039-strips /tmp/ahc039.csv
    g++ -std=c++23 -O2 tests/manhattan_max_transform_test.cpp -o /tmp/ahc039-transform-test
    /tmp/ahc039-transform-test
    python scripts/bundle.py problems/ahc039/main.cpp -o /tmp/ahc039-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc039-single.cpp -o /tmp/ahc039-single
