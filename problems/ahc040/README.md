# AHC040: Packing Uncertain Rectangles

[Official statement](https://atcoder.jp/contests/ahc040/tasks/ahc040_a) /
[generator and interactive tester](https://img.atcoder.jp/ahc040/RGoXy7re.zip).
3 s, 1024 MiB. Each query starts empty and places a strictly ordered
subset, with rotation, slide direction and a preceding reference. Noisy
dimensions and bounding-box measurements are available to the solver.
Absolute score (minimize) is the best query's width + height + dimensions
of omitted items. The 3000-case final rank uses the sum of
`round(1e9 * per-case-minimum-absolute / own-absolute)` (maximize).

Final #1 **2,981,293,882,216**, chokudai. Recomputed from the
[official case-score archive](https://img.atcoder.jp/ahc_standings/ahc040/result.csv)
using exact integer rounding and per-column minima over 1172 rows;
rank matches [official results](https://atcoder.jp/contests/ahc040/results/json).
[Metadata](https://img.atcoder.jp/ahc_standings/ahc040/input.csv) archive SHA-256
`2e5742a7f526ed42c2d26fa76496924d6bedf507701f0a48e84019fc9ae70fcf`,
score archive SHA-256
`7e3b823a5664c0df52d6f1571914303580a4d6fd6079ad01a01a3a3f644f8af5`.
The public final seed hash in the statement is
`1e93374aa02130a1167c2893f1904b4234a3b517d1e7b9d25022a9125ff3777d`.
Those seeds were not used for this experiment. **Official comparison is
undetermined.**

## Shelf control and optional mixed-slide portfolio

The shelf arm builds short horizontal or vertical chains, resetting at
a noisy estimate of target width. Each query has deterministic orientation
jitter and bounding shape variation, from the stated initial observations
only. Both arms emit exactly T full placements and receive judge responses;
the feedback is not used to update the initial estimates in this slice.

The mixed arm uses the same shelf query every third turn. Otherwise it
greedily checks both slide directions, both orientations and an anchor
shortlist for each rectangle. It estimates the collisions with the current
board to rank each option by bounding perimeter, aspect balance and a
box-area term. Noisy scenarios are clamped onto the true shape range.
After the 2.30 s internal guard it emits cheap shelf fallbacks. The physical
slide itself is valid for true dimensions, even when the estimated
position does not match. The actual collision simulator is specific to
the problem's anchor/slide rule; no new generic header is needed.

| 32 fresh cases each | Shelf mean cost | Mixed mean cost | Mixed W/T/L | Paired mean ratio |
|---|---:|---:|---:|---:|
| Development | 1,160,649.06 | 1,150,764.81 | 14/7/11 | 0.98965 |
| Untouched holdout | 1,123,661.47 | 1,118,625.09 | 10/8/14 | 0.99449 |

Holdout median ratio was 1.0, worst 1.02648. For sigma<5000 the mean ratio
was 0.97899 on dev and 0.98047 on holdout; for higher noise it was
1.00174 and 1.00540. This is only exploratory stratification, not a tuned
decision rule. Keep shelf as default; the mixed arm is optional.
All 128 query sequences had no overlaps and matched official and independent
best-query scoring. Max wall 0.312888 s including the tester. Sources use
the same true geometry and noise schedule, but different placements per
query. [Paired log](benchmark-results.csv).

Next replace a small prespecified share of geometry attempts with
safely-identifiable probes. A horizontal reference chain's bounding
width is exactly a sparse sum of selected original dimensions, independent
of its height. Use overlapping chains to update the initial estimates with
regularized least squares, maintain uncertainty, and compare against the
same query-stage split and both unchanged geometries. Do not spend final
cases to tune the estimator.

## Reproduction

From repository root, `ATCODER_TOOLS` points to official `target/release`
after `cargo build --release --locked`.

    g++ -std=c++23 -O3 problems/ahc040/main.cpp -o /tmp/ahc040-shelf
    g++ -std=c++23 -O3 -DAHC040_MIXED=1 problems/ahc040/main.cpp -o /tmp/ahc040-mixed
    python problems/ahc040/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/tester" /tmp/ahc040-shelf /tmp/ahc040-mixed /tmp/ahc040.csv
    g++ -std=c++23 -O2 tests/ahc040_slide_test.cpp -o /tmp/ahc040-slide-test
    /tmp/ahc040-slide-test
    python scripts/bundle.py problems/ahc040/main.cpp -o /tmp/ahc040-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc040-single.cpp -o /tmp/ahc040-single
