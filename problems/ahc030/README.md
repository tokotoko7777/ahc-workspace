# AHC030: Polyomino Mining

[Official statement](https://atcoder.jp/contests/ahc030/tasks/ahc030_a) and
[Rust interactive tester / generator](https://img.atcoder.jp/ahc030/awGA15Om_v3.zip).
Time limit 3 s, memory limit 1024 MiB, up to 2N² operations. A single exact
drill costs 1 and a k≥2 survey costs 1/√k. A wrong support guess costs 1;
the correct guess is free. Absolute score for successful recovery is
`round(1e6*max(cost,1/N))` (minimize), else 1e9. Ranking uses the sum over
3000 system tests of `round(1e9*best_contestant_absolute/my_absolute)`
(maximize; illegal/timeouts contribute zero). The published final seed file
is named on the statement with SHA-256
`45933552af3c557ff53d4e1397ef9fe10cedea89f10fe8140ceb489569dacea2`.
It was not used for experiments.

Final rank 1: **2,140,937,483,257 relative**, from the rank/score CSV of the
[AtCoder official scores archived by ALE-Bench](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc030.zip)
(zip SHA-256 matching the publisher LFS pointer:
`d276d9c5f69390704a2d1692015273b0062a6b04372e9ec35c0c3cb8e05aa1c2`).
Published final seeds alone do not provide the contest’s case-by-case
relative reference minima. Fresh-input absolute results here **do not
determine an official rank comparison**.

## Exact drilling

The control scans cells until it has observed all known total oil mass.
The active solver tracks every translation of each shape separately.
A cell's upper count is the number of shapes that can cover it; its lower
count is the number that must cover it. At a drilled cell, if the measured
count equals either bound, filter all optional shapes to avoid/cover it as
required. Iterate these implications to a fixed point. A known count need
not name the contributing shapes; the solver never guesses an attribution
from a count between its bounds.

Choose the next exact drill by the sum of single-shape Bernoulli coverage
variances under uniform remaining translations, with a small preference
for ambiguous support. This variance is only a selection heuristic, not a
posterior calculation. Stop with a proof: every cell’s presence is decided,
or the sum of one count bound is the total oil mass. If the 2.30 s guard
trips, scan the remaining cells; this never needs more than N² total
drills, no repeated digs, and one final answer.

| 40 fresh cases each | Scan mean cost | Active mean cost | W/T/L | Paired mean cost ratio |
|---|---:|---:|---:|---:|
| Development | 233.525 | 78.975 | 40 / 0 / 0 | 0.31185 |
| Untouched holdout | 205.725 | 67.825 | 40 / 0 / 0 | 0.30432 |

All 160 official transcripts recovered the exact support on the first guess.
An independent replay validates coordinate uniqueness, operation count,
correct support from hidden shapes and placements, and cost. Worst measured
wall time, including the tester, was 0.038554 s. Branch defaults active
drilling. No generic library addition: translation-specific bookkeeping is
local, and plain `std::bitset<400>` supports the filtering efficiently.

Next try cheap noise-aware grouped surveys on the surviving placements:
score selected survey masks by the variance of per-translation intersections
rather than by pointwise variance; maintain a soft ranking only (never
eliminate a true placement merely because a noisy survey disagrees).
Use a verified exact drilling or known-mass fallback. Compare with fresh
paired cases, include failures and high-noise/many-shape stratification.

From repository root (`ATCODER_TOOLS` is official Rust `target/release`):

    g++ -std=c++23 -O3 -DAHC030_PROPAGATE=0 problems/ahc030/main.cpp -o /tmp/ahc030-scan
    g++ -std=c++23 -O3 problems/ahc030/main.cpp -o /tmp/ahc030-active
    python problems/ahc030/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/tester" /tmp/ahc030-scan /tmp/ahc030-active /tmp/ahc030.csv
    g++ -std=c++23 -O2 tests/ahc030_fields_test.cpp -o /tmp/ahc030-fields-test
    /tmp/ahc030-fields-test
    python scripts/bundle.py problems/ahc030/main.cpp -o /tmp/ahc030-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc030-single.cpp -o /tmp/ahc030-single

[Paired results](benchmark-results.csv) include dimensions and epsilon.

## Optional survey prior

`-DAHC030_SURVEY=1` tries 14 cheap rectangle/stripe surveys before any exact
drills. Rank sampled survey masks by moment-matched signal variance per cost:
sum the variance of *intersection sizes* per shape, then compare it to the
judge noise. For each translation update a soft weight using a tempered
Gaussian likelihood with other shapes marginalized by their first two
moments. Exact drilling ranks cells using the new weights, but filtering,
termination, and fallback still use the identical exact-count proof.
Weights cannot eliminate a candidate, even at high noise.

| 40 new cases each | Exact mean cost | Survey mean cost | Survey W/T/L | Paired mean ratio |
|---|---:|---:|---:|---:|
| Development | 55.15000 | 56.67805 | 16/0/24 | 1.08575 |
| Untouched holdout | 69.90000 | 69.40362 | 24/0/16 | 1.02022 |

Holdout median ratio 0.97279; at eps≤0.1 it was 0.94044 (11 cases), but
that subgroup was not consistently improved in development (paired mean
1.03390). At eps>0.1 the holdout paired mean ratio was 1.05512. The weak
inconsistent effect does not justify changing the default or a final run
for the survey variant. All 160 official transcripts and independent replay
agreed, and worst wall time including tester was 0.078703 s.
[Data](survey-results.csv) includes per-case dimensions/eps/phase cost.

    g++ -std=c++23 -O3 -DAHC030_SURVEY=1 problems/ahc030/main.cpp -o /tmp/ahc030-survey
    python problems/ahc030/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/tester" /tmp/ahc030-active /tmp/ahc030-survey /tmp/ahc030-survey.csv --survey

Next explore **joint** feasible placement hypotheses after a small number of
exact queries. A soft posterior or search pool can find a likely support but
must keep separate exact feasibility bookkeeping. Use targeted surveys to
distinguish plausible full assignments; validate candidates through an
exact or sufficiently favorable answer guess and retain the scan fallback.
Review false-confidence cases on a new development split.
