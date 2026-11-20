# AHC035: Breed Improvement

[Official statement](https://atcoder.jp/contests/ahc035/tasks/ahc035_a) /
[official Rust generator and interactive tester](https://img.atcoder.jp/ahc035/F5dI2O6U.zip).
2 s, 1024 MiB. In ten rounds, submit a different seed in each of 36 cells.
For each adjacency, the next seed independently takes each component from
one parent with probability 1/2. Only the current vectors are available.

Each case scores `round(1e6 * bestFinalSum / initialComponentCeiling)`.
The denominator is the sum of the 15 separate initial component maxima;
the final score is the absolute sum over 300 cases, no rerun. Final #1
**274,272,291**, peroon from [official results](https://atcoder.jp/contests/ahc035/results/json).
Direct official case-score/metadata archive endpoints returned HTTP403.
The number is from AtCoder's rank/score CSV packaged in the
[ALE-Bench archive](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc035.zip)
with LFS SHA-256 `e42f0a604bb873d05845c8db311d31074d9e4e987a9235b7bd771e1dd6c0913b`.
We have not established that the generator's 100 starter cases are the
same as the 300 contest cases. **Official comparison remains undetermined**.

## Baseline and complementary parents

The control keeps the 36 highest-total current vectors and plants the
strongest on the highest-degree grid vertices. The new variant starts from
the same rule each turn, then anneals swaps among both planted locations
and unused candidates. Its score uses the following terms:

- A contribution from the average total vector of the prospective children.
- A smooth upper-tail proxy: for each edge the exponential moment of a
  child sum is the product of per-component two-point exponential moments.
  The log of the sum of these moments emphasizes good complementary parents,
  not just the sum of the two parent totals. This is not the actual expected
  maximum and does not assume one fixed crossover outcome.
- Early generations penalize the exact marginal probability of losing each
  current coordinate maximum in the next generation. If any edge has
  two carriers, that event has probability zero; otherwise each one-carrier
  edge independently fails with probability 1/2. A larger gap to the next
  distinct value receives a higher penalty.

An update recomputes only incident edges, deduplicating their common edge.
This state/score logic is problem-specific; short arrays and masks suffice.
A debug mode checks the incremental counts and moment total against a
fresh pass over all 60 edges. The solver never accesses hidden masks.

| Cases | Degree control mean | Complementary mean | W/T/L | Median gain |
|---|---:|---:|---:|---:|
| Development (36) | 720,290.53 | 852,186.06 | 36/0/0 | 134,070.5 |
| Untouched holdout (36) | 722,317.89 | 857,950.08 | 36/0/0 | 131,131.5 |

On holdout, the ratio of remaining component ceiling to initial ceiling
averaged 0.78560 vs 0.95101. This measures diversity preserved, not the
actual score. All 144 interactive transcripts were legal and matched
official score and independent masked replay. The two arms use the same
case's masks, but an edge may have different parents. Maximum wall time
including tester 0.161581 s; internal guard 1.55 s. New variant is branch
default. [Paired results](benchmark-results.csv).

Next use the exact distribution of every pair's child total in the last
few generations, instead of an exponential-tail proxy. For a fixed
planting, multiply the per-edge CDFs to get the max-child CDF; use
tokenized nonzero values and zero counts for fast delta comparisons.
Retain the early diversity objective, and test on fresh cases before
promoting that late variant.

## Reproduction

From the repository root (`ATCODER_TOOLS` points to the official tools'
`target/release`, compiled with `cargo build --release --locked`):

    g++ -std=c++23 -O3 -DAHC035_TAIL=0 problems/ahc035/main.cpp -o /tmp/ahc035-degree
    g++ -std=c++23 -O3 problems/ahc035/main.cpp -o /tmp/ahc035-tail
    python problems/ahc035/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/tester" /tmp/ahc035-degree /tmp/ahc035-tail /tmp/ahc035.csv
    python scripts/bundle.py problems/ahc035/main.cpp -o /tmp/ahc035-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc035-single.cpp -o /tmp/ahc035-single

Use `-DAHC035_VERIFY` in a diagnostic build for full-vs-delta checks.
