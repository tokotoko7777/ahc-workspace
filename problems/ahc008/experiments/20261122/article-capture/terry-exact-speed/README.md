# Exact fast stationary handoff

Pinned AHC008 independent TERRY-method benchmark source SHA256
`54f2b1ecc90820fcc5c99bc4e3cb8ddfe06c5952bf45851bf9d1a21e49341ae9`.
Once every FIFO build queue is actually empty and the current column
multiset equals the original cyclic target column multiset, the original
bottleneck assignment has a zero-distance perfect solution. Its ETA is
zero, the physical target column for each person equals their position,
and it issues no handoff movement. Return that fact without rebuilding
the wall-completion distance tables and threshold flow. For all other
situations preserve the exact old algorithm and tie order. No model
coefficients, terrain, patrol, capture, switch gate or search budget
changed. This assignment short-circuit is possible because the current
instance's zero-cost edges are complete bipartite within each column;
it is not safe to replace a generic zero-cost flow unless the zero-cost
edge partition is proven.

## Reproduce

Use the previously frozen standalone **locally** with the authorized model
data. The derived stand-alone also carries those private coefficients,
so neither compiled binary nor flattened source is bundled.
Source regenerated SHA256
`71b0edebbf87e873543f6438786d68d8b1130ff69975605318ed8b18507fdad7`.

    python ahc008_terry_speed/make.py ahc008_terry_exact_root/submission.private.cpp ahc008_terry_speed/submission.private.cpp
    g++ -std=c++23 ahc008_terry_speed/submission.private.cpp -o ahc008_terry_speed/solve
    cc -O2 ahc008_terry_speed/time.c -o ahc008_terry_speed/time
    python ahc008_terry_speed/bench.py ahc008_focus_assets/tools/target/release/tester ahc008_focus_assets/tools/in ahc008_terry_exact_root/practice100.csv ahc008_terry_exact_root/standalone ahc008_terry_speed/solve ahc008_terry_speed/practice.speed.csv

Repeated serial per case with alternating order, use solver-only CPU from
wait4. For this equivalence/performance audit only, both arms receive the
same completed 275-turn pair count from the previous verified ledger,
so they do the same search. Check exact original command SHA and exact
official score on every invocation. This test does not use the case or
count at runtime for any intended submission. In ordinary wall-clock mode
this optimization might change the number of paired endgame simulations;
matching fixed-work commands is not a claim of identical scored output
when run against a clock with extra headroom.

## Verified fixed-work result

All 400 exact official tester runs (two executable arms on each case across
both fixed 100s) matched the prior independently replayed hashes and official
scores. Both 100-case scores, all turns, all legal statuses and terminal
simulation workload are identical in this audit.

| Same paired workload | Old solver CPU | Fast solver CPU | Reduction |
|---|---:|---:|---:|
| Official practice100 | 87.943815 s | 84.264207 s | 4.184% |
| Independent generated100 | 101.557045 s | 96.581030 s | 4.900% |
| Practice subset without terminal rollouts (86) | 50.888107 s | 48.656817 s | 4.385% |
| Generated subset without terminal rollouts (81) | 51.676443 s | 48.787622 s | 5.590% |

Practice max local tester wall time 2.7391→2.7102 s;
generated 2.7572→2.7418 s. These are serial local fixed-work numbers,
not an AtCoder guarantee. The earlier scoring reference remains the frozen
two default-budget benchmarks. No actual score increase, merge or
submission claimed. This is a separate speed-preserving worktree with
collapsed zero-distance handoff assignments only.
