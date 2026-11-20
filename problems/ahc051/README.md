# AHC051 — Probabilistic Waste Sorting

- [Official problem and scoring](https://atcoder.jp/contests/ahc051/tasks/ahc051_a)
- [Official generator and visualizer](https://img.atcoder.jp/ahc051/jdd9gfQC.zip)
- [Official final per-case score matrix](https://img.atcoder.jp/ahc_standings/ahc051/result.csv)
  (SHA-256 `7c1b931d4c62deb3fd63d1a3401e398e9ecba0bffdc2d3439d6f58b2bf0ec838`)
- [Official final case list](https://img.atcoder.jp/ahc_standings/ahc051/input.csv)
  (SHA-256 `1e3708bf9507f664c83dc1c4ac4cb4ec065739eb712be1f1ddce65c0c43e51d8`)
- [Official ranking metadata](https://atcoder.jp/contests/ahc051/results/json)

On each case the absolute score is the rounded mean probability of incorrect
processing multiplied by 10⁹ (smaller better). Sum the 2000 individually rounded
`10⁹ * min_abs_case / abs_case` values over legal timed cases to get the
final relative score (larger better). Across the official archive's 1183
rows, integer rounding `(2*10**9*min_abs_case + abs_case)//(2*abs_case)`
identifies eijirou's historical winning score **1,721,504,233,142**, agreeing
with the public rank metadata. If a new evaluation sets a new case minimum,
the contestant and historical leader must both use this new minimum for
a joint comparison. No historical input cases were run in this experiment;
the historical rank remains **unjudged** for this solver.

## Implementation

Both arms create the same five candidate tree geometries. To add a terminal,
replace an arc ending at a leaf with a three-edge splitter insertion, checking
all three arcs for forbidden closed intersections. Only endpoint-sharing arcs
are exempt. Every insertion preserves a rooted directed tree by construction.
For each tree, initialize sorter types from the gap in mean routing probabilities
between its two leaf sets and assign physical processors with Hungarian matching.

The fit arm also alternates exact single-sorter refits with reassignment. For
fixed assignment and other sites, let `F[c]` be probability that kind `c`
reaches the edited site and `A[c]`, `B[c]` the correct-processing probabilities
from its two children. The objective is affine in the site's branch probabilities,
with vector `F*(A-B)`. Evaluate each available type and both orientations using
[`best_binary_split`](../../library/binary-split-fit.hpp) and keep best exact
whole-tree result. Forward and reverse tree traversal avoids full rescoring
each sorter candidate. The repository's fixed-action-sequence DP does not
directly represent a vertex with independent children and a leaf assignment
updated by matching; geometry and messages stay problem-local.

| Set | Cases | Baseline absolute mean ↓ | Fit absolute mean ↓ | Better / Tie / Worse |
|---|---:|---:|---:|---:|
| Development | 28 | 635,654,296 | 609,971,541 | 25 / 3 / 0 |
| Holdout | 28 | 675,522,621 | 640,624,514 | 27 / 1 / 0 |

Holdout median paired absolute-score ratio 0.94965; median improvement
34,599,142. All 112 runs used all processors and matched the official visualizer
and independent exact-rational score, closed segment geometry, range/activation,
and acyclicity checks. Max measured solver wall time 0.104 s (2 s limit); fit is
the branch default. [Test data](benchmark-results.csv) uses hashed fresh seeds,
never the official case list.

**Next hypothesis:** the tree's paths cannot reconverge and consume too much
routing confidence near roots. Insert an extra discriminator with two distinct
downstream paths to an existing child pair; test planarity and the exact
probability gain of this small DAG gadget against the same allotted time spent
on additional tree geometries.

```sh
p=problems/ahc051
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC051_FIT=0 "$p/main.cpp" -o "$d/base"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/fit"
python "$p/benchmark.py" /path/to/gen /path/to/vis "$d/base" "$d/fit" "$d/results.csv"
g++ -std=c++23 -O3 tests/binary_split_fit_test.cpp -o "$d/test"
"$d/test"
```
