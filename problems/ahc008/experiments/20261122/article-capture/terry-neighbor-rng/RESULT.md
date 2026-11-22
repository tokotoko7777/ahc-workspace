# Rejected: bounded sampler arithmetic specialization

In the accepted exact-passive reference, only Normal's uniform draw among
1–4 ordered neighbors was changed. The original rejection range
`(n << countl_zero(n)) - 1` becomes `0x7fffffff + (n==3)*2^30`. This is
the four-entry algebraic identity, not a change of sample law; it preserves
which draws are rejected even for n=1. In the helper unit, a million calls
agreed on value, rejection/draw count and final RNG state. A second test
covered input edges of the sorted multiply-high and acceptance intervals.

The built control and variant were GNU C++23 standalone files without
extra optimization flags. Variant source SHA-256:
`1a4b0d8426d66309ad7f8ed8a2bf0e3f126bf50ae6c6e9d67733de6a5c7d1e6f`.
In 200 sequential alternating official tester runs on 100 official
distributed cases, fixed completed virtual pair counts, the entire action
streams and scores matched the validated practice high for all runs.
Total solver CPU was 69.724328 s → 69.835540 s (+0.1595%, i.e. no speedup).
For the 88 no-rollout cases the corresponding CPU totals are
38.427320 s → 38.757958 s. Max local tester wall 2.7230 → 2.7464 s.
Reject according to GATE.md; do not run on the held panel or replace the
current solver. This does not imply the same arithmetic is slower on
other compilers and CPUs. No live score test was run.

```sh
g++ -std=gnu++23 -O2 ahc008_terry_neighbor_rng/unit.cpp -o /tmp/ahc008-bound-test
/tmp/ahc008-bound-test
python ahc008_terry_neighbor_rng/make.py
g++ -std=gnu++23 ahc008_terry_neighbor_rng/solve.private.cpp -o ahc008_terry_neighbor_rng/solve
python ahc008_terry_speed/bench.py ahc008_focus_assets/tools/target/release/tester ahc008_focus_assets/tools/in ahc008_terry_passive_exact_short/practice.csv ahc008_terry_passive_exact_short/live ahc008_terry_neighbor_rng/solve result.csv
```

Solver and original supplied data are not in this archive.
