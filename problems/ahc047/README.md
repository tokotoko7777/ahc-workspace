# AHC047: Lovely Language Model

[Official statement](https://atcoder.jp/contests/ahc047/tasks/ahc047_a) /
[official generator and scorer](https://img.atcoder.jp/ahc047/cHmFekjC.zip).
2 s, 1024 MiB. A 12-state labelled Markov chain emits the initial
state's label and then transitions, for 1,000,000 characters. Each
row is a legal nonnegative integer percentage vector summing to 100.
Per-case score rounds weighted probabilities of seeing each requested
substring at least once. The 150-case absolute sum decides the rank;
no post-contest rerun.

Rank 1 itigo by [official results](https://atcoder.jp/contests/ahc047/results/json).
Exact top score not verified: [standings](https://atcoder.jp/contests/ahc047/standings)
redirects to sign-in, case CSV endpoints returned 403, and the
expected ALE-Bench official ranking archive is not available (404).
The official tools contain 100 starter cases, not proven identical
to the 150 actual cases. **Official comparison undetermined.**

## Deterministic cycle and stochastic alternatives

The control makes a deterministic cycle for the highest-value word.
The variant keeps that fallback, then uses four starts: three slightly
randomized versions of the most valuable cycles, and a weighted-pair
transition model with two copies of every letter. Improve the integer
transition rows by reallocating probability mass and swapping state
labels. The fast proxy roughly estimates stationary word beginning
rates, converts each to `1-exp(-L*p)`, and intentionally does **not**
treat that as an exact occurrence probability: word overlaps and
long-run clustering can change actual outcomes.

Select between complete models by the exact finite-length probability.
The [Markov pattern library](../../library/markov-pattern-probability.hpp)
builds only reachable pairs of chain vertex and KMP matched-prefix
length. Remove edges that first encounter a match; matrix-power the
remaining substochastic paths. Missing probability mass is precisely
the probability of a hit. The existing matrix utility does not build
or prune this product graph. The random optimizer uses an internal
1.50 s stop and never outputs an incomplete model.

| 24 fresh cases each | Cycle mean | Mixture mean | W/T/L | Median gain |
|---|---:|---:|---:|---:|
| Development | 13,671.67 | 23,499.75 | 24/0/0 | 10,298 |
| Untouched holdout | 13,864.71 | 21,656.67 | 24/0/0 | 5,523 |

All 96 outputs matched the official score and separately rebuilt NumPy
matrix extrapolation, plus legal labels and full row sum checks.
Maximum wall time 0.097127 s. Branch defaults mixture.
[Results](benchmark-results.csv). No final seeds were used.

Next jointly construct paths for several high-value words over an
explicit shared-state template. Beam-select mappings of repeated
letters to states using existing edges/rare bottleneck probability;
then fit the integer row masses. Contrast with additional iterations
from the same four starts at similar wall time. Holdout cases remain
untouched during that experiment.

## Reproduction

Build official tools from their directory via `cargo build --release --locked`.
From repository root (`ATCODER_TOOLS` points to their `target/release`,
Python with NumPy):

    g++ -std=c++23 -O3 -DAHC047_MIX=0 problems/ahc047/main.cpp -o /tmp/ahc047-cycle
    g++ -std=c++23 -O3 -DAHC047_PATHS=0 problems/ahc047/main.cpp -o /tmp/ahc047-mixture
    OPENBLAS_NUM_THREADS=1 python problems/ahc047/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc047-cycle /tmp/ahc047-mixture /tmp/ahc047.csv
    g++ -std=c++23 -O2 tests/markov_pattern_probability_test.cpp -o /tmp/ahc047-prob-test
    /tmp/ahc047-prob-test
    python scripts/bundle.py problems/ahc047/main.cpp -o /tmp/ahc047-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc047-single.cpp -o /tmp/ahc047-single

## Shared word-path initialization

Keep the exact-scored four original starts. Two added starts differ only
in labels: two copies per character or a fixed base copy plus six
copies assigned by high-value word demand. For each of ten desirable
words, use a Viterbi scan over compatible copies, favoring edges already
supported by other words and penalizing competition within an outgoing
row. Add its route to fractional edge weights and round with largest
remainders to integer row sums of 100. From there run the **same** 4,500
proposal budget per start as before and adopt only an exact-score winner.

A resource control adds the same two restarts from shuffled labels
on the existing weighted matrix, rather than giving the variant more
annealing. All three arms otherwise share the four original starts.

| 24 fresh cases each | Four starts | Six, random extra | Six, mapped paths |
|---|---:|---:|---:|
| Development mean | 23,054.25 | 24,121.33 | 26,979.54 |
| Untouched holdout mean | 21,549.25 | 22,646.17 | 26,281.33 |

Against the extra-restart control, holdout 15/6/3 (W/T/L), median +1131.5.
Median solver wall 0.08805s vs0.08673s (random extra vs mapped) and
overall max0.114058s. All 144 outputs matched official and independent
exact scoring. The branch defaults to the mapped paths; no final data.
[Paired log](shared-path-results.csv).

The compatibility scoring and integer row fitting are local model
choices, not a new generic component. The synthetic test replays both
initializers, checking supported word routes and row sums on random inputs.
Next alternate between reassigning just the top few word routes and
retuning the row probabilities, using the current exact score as the
outer quality filter. That can escape greedy insertion order constraints.

    g++ -std=c++23 -O3 problems/ahc047/main.cpp -o /tmp/ahc047-shared
    g++ -std=c++23 -O3 -DAHC047_PATHS=2 problems/ahc047/main.cpp -o /tmp/ahc047-extra
    OPENBLAS_NUM_THREADS=1 python problems/ahc047/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc047-mixture /tmp/ahc047-shared /tmp/ahc047-shared.csv --extra-starts /tmp/ahc047-extra
