# AHC007: adaptive sampling with diversity correction

This is the low-runtime independent experiment. It does not replace the
repository default or claim a leaderboard result. The official public tools
archive contains 150 examples; its set is not presumed identical to the
original contest inputs.

When we receive a cost, contract the accepted forest. Draw random future
lengths from the required integer uniform distributions, then find the
minimum bottleneck of a future-only route between the current edge's two
components in each draw. Only revealed weights are read and each decision
is flushed. Take a compulsory edge when no alternative route exists; skip
cycles unconditionally. The maintained invariant is that accepted and
unrevealed edges together connect the entire graph.

Rather than using only the average bottleneck, add
`0.15 * standard_deviation * Pr(distinct witness edges)`. The latter
probability is estimated from distinct draw pairs. The witness is a maximum
weight edge on a minimum-bottleneck route. At a weight tie, it need not match
the original Kruskal implementation's identity. Different witnesses are a
cheap signal for future choices that a hindsight completion can make but
the online policy cannot make. This is a heuristic, not a bound.

At most 96 stratified scenarios are calculated, starting with 12 then doubling
only near the predicted threshold. The stop band is 2.5 estimated standard
errors plus 1.5% of the estimated mean. This band is a heuristic; correlated
strata and adaptive sampling do not warrant an i.i.d. interval interpretation.
A clock guard cuts the budget to eight and then one scenario, but never
bypasses the invariant or the flush. The code declares its required
optimization in-source and only uses the C++23 standard library.

## Frozen measurements

| Input | A/B comparison | Reference score | Adaptive risk | Wins | max lockstep wall / solver CPU |
|---|---|---:|---:|---:|---:|
| 150 public archive examples | 48 stratified, same mean+diversity correction | 14,240,253,908 | **14,247,857,888** | 78/150 | 0.338 s / 0.318 s |
| 150 new official-generator cases | incumbent 24-scenario mean | 14,156,200,691 | **14,224,199,058** | 118/150 | 0.348 s / 0.303 s |

Public: +6,123,947 over the provided contest reference 14,241,733,941;
unseen: +67,998,367 over the incumbent, but under that reference by
17,534,883. The public sets were identical across compared variants; the
independent original benchmark used a separate frozen range (27,155,000
through 27,155,149). Both scoring paths agreed on all 600 legal runs.
The adaptive solver used 4,690,481 draws across 245,491 noncycle queries on
the public set, with 224,973 early stops. It did not reach the clock guard.
The cleaned submission was checked against the frozen adaptive executable
for identical outputs on all public 150 cases.

## Reproduction

Build official `gen` and `vis` with
`cargo build --release --locked --bin gen --bin vis` from the
[official archive](https://img.atcoder.jp/ahc007/a855b6a456c9892b747e147001b0f89.zip).
Archive SHA256:
`50d13ca564ada8e06e4aea2b797c2c7e19fdc3cecd41b3b48222cc9f35cc76f0`.

From repository root:

```sh
d=problems/ahc007/experiments/adaptive_risk
mkdir -p build/ahc007-risk
python3 scripts/bundle.py "$d/main.cpp" -o build/ahc007-risk/submission.cpp
cmp "$d/submission.cpp" build/ahc007-risk/submission.cpp
g++ -std=c++23 build/ahc007-risk/submission.cpp -o build/ahc007-risk/adaptive
g++ -std=c++23 "$d/weighted_bottleneck_witness_test.cpp" -o build/ahc007-risk/test
build/ahc007-risk/test
python3 problems/ahc007/benchmark.py "$public_in" build/ahc007-risk/public \
 "$vis" build/ahc007-risk/adaptive build/ahc007-risk/adaptive
```

A/B raw scores and times for both input sets are in
[`measurements.csv`](measurements.csv). Timings used a fixed CPU with independent
official and integer checks; machine-to-machine timing variation is expected.
