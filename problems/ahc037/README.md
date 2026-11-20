# AHC037: Soda

[Official statement](https://atcoder.jp/contests/ahc037/tasks/ahc037_a) /
[official tools](https://img.atcoder.jp/ahc037/WneGTzJP.zip). 2 s, 1024 MiB,
1000 targets and at most 5000 operations. A monotone operation
`(x,y) -> (a,b)` costs `a+b-x-y`. The absolute score is
`round(1e6 * N * maxCoordinate / (1 + totalCost))`. The 150 contest cases
are summed, with no post-contest rerun.

Rank #1 was Rafbill by [official results](https://atcoder.jp/contests/ahc037/results/json).
Exact top score is unverified: [standings](https://atcoder.jp/contests/ahc037/standings)
requires login, direct official per-case archive endpoints returned HTTP403,
and the rank-only AtCoder archive is not available for AHC037 in ALE-Bench
(its expected ZIP returns 404). The official tools have 100 starter
inputs, not established as the 150 contest cases. **Official comparison
remains undetermined.**

## Terminal-only control vs shared junctions

For every target in ascending coordinate sum, the control picks the
dominated earlier target with greatest sum, or the origin. This is the
cheapest parent assignment when only the supplied terminals are allowed.

The variant works backwards: a pair of unresolved subtrees may be joined
at its coordinatewise minimum (shared reachable corner). Pick the pair
with greatest corner-coordinate sum, preferring an existing endpoint in
the event of a tie. Store the resulting binary tree and walk from its
root outward to print operations in dependency order; omit zero-cost
same-coordinate loops. At most 2N−1 edges. Keep the control if the
variant's actual cost does not improve. Arbitrary score ties are broken by
stable node IDs rather than input-specific branching.

The active pair set changes after every join. To avoid a cubic rescan,
the general [agglomerative-pair-merge helper](../../library/agglomerative-pair-merge.hpp)
caches pair ranks, removes stale edges lazily and returns the merge tree.
The O(n²) candidate store is appropriate at N=1000 but not ideal for huge
sets; no grid or geometry assumptions enter the helper. A deterministic
unit test compares the full merge sequence against repeated enumeration.

| 28 fresh cases each | Terminal mean | Junction mean | W/T/L | Paired cost ratio |
|---|---:|---:|---:|---:|
| Development | 22,779,093.11 | 35,997,057.68 | 28/0/0 | 0.63282 |
| Untouched holdout | 22,842,660.79 | 35,832,854.21 | 28/0/0 | 0.63749 |

All 112 outputs matched official scoring and independent reachability
and exact cost checks. Maximum 1696 operations and worst wall time
0.421438 s. The branch defaults to shared junctions.
Representative starter-input peak RSS: 25,088 KiB.
[Individual generated-case results](benchmark-results.csv).

Next reuse the created candidate junctions but choose better *parents*
independently among all dominating retained nodes, not only their original
merge ancestor. Prune unused auxiliaries and contract single-child
auxiliaries without raising cost, then try resynthesizing shared corners
where the new tree still branches expensively. Measure on new seed splits.

## Reproduction

From the repository root (build the official tools with
`cargo build --release --locked`, set `ATCODER_TOOLS` to their
`target/release` path):

    g++ -std=c++23 -O3 -DAHC037_JOIN=0 problems/ahc037/main.cpp -o /tmp/ahc037-terminal
    g++ -std=c++23 -O3 problems/ahc037/main.cpp -o /tmp/ahc037-join
    python problems/ahc037/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc037-terminal /tmp/ahc037-join /tmp/ahc037.csv
    g++ -std=c++23 -O2 tests/agglomerative_pair_merge_test.cpp -o /tmp/ahc037-merge-test
    /tmp/ahc037-merge-test
    python scripts/bundle.py problems/ahc037/main.cpp -o /tmp/ahc037-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc037-single.cpp -o /tmp/ahc037-single

## Optional matched-set reparent/prune

Rebuild parents on the exact same unique terminal-plus-junction set:
initially retain the existing cheapest incoming edge, then search earlier
nodes in coordinate-sum order for a higher-sum dominating parent. Compare
two ways of scanning the same set: allow newly assigned parents only
from terminals, or also allow synthetic junctions. Both use the same
parent check and same downstream passes: remove auxiliary leaves and
contract single-child auxiliary nodes. The original unmodified merge
result is the third control and a nonworsening fallback.

All three tied on all **28 development and 28 untouched holdout** cases,
with scores 35,801,010.18 and 35,988,519.46 respectively. Holdout median
wall times 0.2354 / 0.2350 / 0.2481 s (frozen / terminal parent search /
all parent search); max 0.460654 s. All 168 official scores agreed with
independent replay. The operation and junction counts did not change.
No default change; [reparent trace](reparent-results.csv).

This plateau is consistent with the original max-meet order already
selecting strong parents among nodes it made: a cheaper existing node
dominated by a child would offer a high-priority meet with that child
before it disappears in a more expensive merge. Do not conclude that
the junction set itself is optimal. Try pooling junctions from
different merge priorities (e.g. perturb relative coordinate weights)
or synthesizing candidates outside that one tree, so the reparenting
actually accesses new options. No public/final test seeds used.

    g++ -std=c++23 -O3 -DAHC037_REPARENT=1 problems/ahc037/main.cpp -o /tmp/ahc037-reparent
    g++ -std=c++23 -O3 -DAHC037_REPARENT=2 problems/ahc037/main.cpp -o /tmp/ahc037-terminal-parents
    python problems/ahc037/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc037-join /tmp/ahc037-reparent /tmp/ahc037-reparent.csv --reparent-control /tmp/ahc037-terminal-parents
    g++ -std=c++23 -O2 tests/ahc037_reparent_test.cpp -o /tmp/ahc037-reparent-test
    /tmp/ahc037-reparent-test
