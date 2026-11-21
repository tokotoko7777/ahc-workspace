# Optional distinct-symbol prefilter (standalone benchmark)

The scorer intersects the row/column presence sets of all distinct symbols
in the query, then tests the surviving lines in ascending original order,
using the unchanged cyclic-position intersection. Signatures are prepared
once per string outside timings. Both masks are updated after every edit.
The production scorer and all fill and deadline behavior remain untouched.

```sh
g++ -std=c++23 tests/toroidal_symbol_masks_test.cpp -o test && ./test
g++ -std=c++23 problems/ahc004/experiments/line_prefilter/bench.cpp -o bench && ./bench
```

The literal differential test passed 2,310 updates, across side lengths 1–20.
The benchmark uses 20 independently drawn 20x20 boards per cell, five
alternating baseline/variant pairs, 16 rounds of 400 words per board, and
optional 20 unique reversible cell changes per round. Reported median
ratios are baseline/variant, greater than one is faster:

| Occupancy | Word length | Queries | Queries + mutations |
|---|---:|---:|---:|
| 25% | 4 | 3.17x | 3.24x |
| 25% | 7 | 3.87x | 3.73x |
| 25% | 11 | 3.95x | 3.66x |
| 65% | 4 | 1.26x | 1.23x |
| 65% | 7 | 1.56x | 1.54x |
| 65% | 11 | 1.64x | 1.74x |
| 100% | 4 | 0.85x | 0.87x |
| 100% | 7 | 0.94x | 0.99x |
| 100% | 11 | 1.05x | 1.01x |

Construction alone costs 18–29% more across these densities. With
construction plus updates but no queries, overhead was 13–17%.
Recommendation: do not blanket-port into the already-fast scorer, especially
not in end-stage dense-board use; consider a sparse-score-only opt-in later
if terminal profiling shows enough total time there. No official cases were
used in these tests.
