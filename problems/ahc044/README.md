# AHC044: Cleaning Up

[Official statement](https://atcoder.jp/contests/ahc044/tasks/ahc044_a) /
[official generator and scorer](https://img.atcoder.jp/ahc044/PnJFT8lu.zip).
2 s, 1024 MiB. Starting at employee 0, the odd/even outgoing choice
alternates independently for each employee. Score is `1,000,000 - E`,
where E is total absolute visit-count error over the 500,000 assignments.
The final absolute total sums 150 cases, no rerun.

Final #1 **149,807,848**, Rafbill per
[official results](https://atcoder.jp/contests/ahc044/results/json).
Official raw/metadata CSV endpoints return HTTP403. The number comes from
AtCoder's rank/score CSV in the
[ALE-Bench archive](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc044.zip),
verified against the LFS SHA-256
`52b6cb057545c468eb48910bf634170e1a89735bb3505b39cf4e1607c2e67135`.
The 100 official starter cases are not established as the 150 final
cases. **Official comparison undetermined.**

## Weighted residue and exact scoring

Fix one successor per employee into a Hamiltonian cycle. The cycle goes
up by even target ranks and back down by odd ranks to limit the jump
into the smallest targets. This guarantees strong connectivity whatever
the other edges do. In the target-balance approximation, both outgoing
edges from source i contribute `T_i` to incoming weight and everyone
should receive `2*T_i`. The greedy control assigns free edges from
largest T first to the largest current deficit.

Refine the free edges via single redirects and two-source swaps.
Each accepted update changes only two entries of the incoming weight.
Vary a squared penalty on badly imbalanced small targets; use the
proxy to search, but select saved candidates using the **complete**
alternating-edge simulation over 500,000 steps. Keep the frozen greedy
answer if none truly improve. These two notions of error are not the
same: steady visit counts can amplify small local residuals over
many transitions. Both state rules are problem-specific and use arrays.

| 24 new cases each | Greedy mean error | Refined mean error | W/T/L | Paired mean error ratio |
|---|---:|---:|---:|---:|
| Development | 67,799.08 | 43,905.67 | 9/15/0 | 0.82339 |
| Untouched holdout | 55,037.42 | 41,167.08 | 9/15/0 | 0.87820 |

Holdout median gain was zero; the fallback accounts for the ties. All
96 outputs matched official score and independent full simulation.
Worst wall 0.160253s (the internal search guard is 1.55s). The branch
defaults to the nonworsening refinement. Pairs are in
[benchmark-results.csv](benchmark-results.csv).

Next use a graph-aware proxy rather than only incoming target balance:
estimate steady count sensitivity to small updates by solving stationary
influence equations around a fixed incumbent, and validate promising
arcs using full rotor simulation. Separately consider rewiring the
cycle to help near-zero quotas, with a reachability certificate and
the same best-complete fallback. Use new seed splits.

## Reproduction

From repository root, `ATCODER_TOOLS` is the official `target/release`
after `cargo build --release --locked`.

    g++ -std=c++23 -O3 -DAHC044_ANNEAL=0 problems/ahc044/main.cpp -o /tmp/ahc044-greedy
    g++ -std=c++23 -O3 problems/ahc044/main.cpp -o /tmp/ahc044-refine
    python problems/ahc044/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc044-greedy /tmp/ahc044-refine /tmp/ahc044.csv
    python scripts/bundle.py problems/ahc044/main.cpp -o /tmp/ahc044-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc044-single.cpp -o /tmp/ahc044-single

Use `-DAHC044_VERIFY` to check residual updates against full recomputation
during search. Do not include that flag in the timed solver.
