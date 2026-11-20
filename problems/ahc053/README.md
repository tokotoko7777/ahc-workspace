# AHC053 — Random Sum Game

[Official problem](https://atcoder.jp/contests/ahc053/tasks/ahc053_a) and
[official interactive tester / generator](https://img.atcoder.jp/ahc053/Q405bDmv.zip).
The fixed 150 contest cases use the sum of per-case
`round((20 - log10(1 + sum_j abs(S_j-B_j))) * 50,000,000)` (larger better).
There is no separate system test. Perfect score would be 150,000,000,000
if all 150 cases have zero error. This is a bound, not a claim that it was
achieved. [Official ranked result](https://atcoder.jp/contests/ahc053/results/json)
identifies yosupo as first, but has no score. The direct official input/results
CSV endpoints return 403, standings requires sign-in, and the ALE rank-only
archive path is unavailable. The final numeric score and exact case inputs
have not been verified; the strict comparison is **unjudged**.

## Fixed deck and assignment

Before the targets are seen, lay out seven disjoint groups of near-uniform cards.
Each provides one card for each target, plus spares. Group zero carries the
main offset; later groups encode signed corrections with positive offsets and
6× narrower radius. Both arms advertise exactly the same deck before reading
any target. At each group, compare independent nearest available cards against
a global sorted minimax assignment (tie-break by total absolute residual).
The latter exploits absolute-distance uncrossing in one dimension, then uses
two dynamic programming passes: it must fix the optimal bottleneck before
minimizing the total. Keeping only the lexicographically best (max, total)
prefix in one DP is not valid because later larger costs can eclipse the prefix
bottleneck and make a smaller prefix total preferable instead.

| Fresh cases | Nearest score ↑ | Global match score ↑ | Better / tie / worse |
|---|---:|---:|---:|
| Dev (32) | 383,627,530 | 387,598,137 | 22 / 0 / 10 |
| Holdout (32) | 383,046,985 | 386,994,135 | 18 / 0 / 14 |

Holdout mean error fell from 2.332e12 to 1.931e12, but median paired score
gain was only 225,134 while 14 of 32 cases regressed. A one-stage minimax
objective does not capture the final multistage error. Keep the baseline
default and retain the global arm as a diagnostic. All 128 runs matched the
official interactive tester and independent output/score replay, max wall
0.0221 s including tester overhead. Fresh cases are reproduced from
[the recorded seeds](benchmark-results.csv), not the contest input set.

**Next hypothesis:** card capacity near zero is the bottleneck when residuals
contract. Redesign the deck rather than tune assignment alone. Put one coarse
card per pile, then use a common pool of low- and high-remainder grids with
several copies and capped tails. Match two-card sums to a residual, prioritizing
scarcity by tracking unused endpoints, and then apply an independent
fine-correction pool. Compare against the same number of cards with uniform
single-pick tiers, using aggregate final error and worst pile separately.
Validate on fresh seeds.

```sh
p=problems/ahc053
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC053_PAIR=0 "$p/main.cpp" -o "$d/base"
g++ -std=c++23 -O3 -DAHC053_PAIR=0 -DAHC053_MATCH=1 "$p/main.cpp" -o "$d/match"
python "$p/benchmark.py" /path/to/gen /path/to/tester "$d/base" "$d/match" "$d/results.csv"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/submission"
g++ -std=c++23 -O3 tests/sorted_bottleneck_assignment_test.cpp -o "$d/test"
"$d/test"
```

## Follow-up: sealed two-card pools

The new 500-card deck uses 80 wide anchors and three independent pools of
140 stratum-jittered denominations. All cards are deterministic functions of
the public first line and a constant PRNG seed, emitted and flushed before
any targets. Anchor matching is the sorted-minimax solver above. Later stages
allocate two cards per target, which allows much higher central density via
combinations without placing all card values in a small band. For the current
unused pool, a two-pointer search computes the closest distinct pair for
each unassigned target. Process the farthest-from-any-pair target first, and
remove both selected cards. No tuning on the five fresh groups below.

| Fresh group, 20 cases each | Nearest ↑ | Global matching ↑ | Paired pool ↑ |
|---|---:|---:|---:|
| Dev A | 383,366,768 | 383,708,766 | 596,162,551 |
| Dev B | 382,378,844 | 383,935,909 | 598,823,574 |
| Holdout A | 381,385,215 | 384,996,296 | 595,085,466 |
| Holdout B | 385,073,204 | 385,560,340 | 595,636,730 |
| Holdout C | 384,145,417 | 386,109,835 | 596,341,624 |

All 100 paired-pool cases beat both controls; all 300 runs agreed with the
official interactive tester and independent replay. The deck bytes were
unchanged across targets within each arm. Error medians fell from the
order of 2×10¹² to 1.1×10⁸. The paired assignment has more work per instance,
but all three arms use the same 500-card capacity and the same 2 s wall
limit, with the paired tester-inclusive maximum 0.0052 s. Paired pools are
the branch default. [Raw measurements](benchmark-pair-results.csv).

Next, before any new holdout, try using the 40 unallocated cards in the finest
pool as dedicated fine grid cards instead. Compare a two-sum assignment with
local exchange/reassignment of its interval endpoints against additional
dedicated fine rounds at equal card capacity.

```sh
p=problems/ahc053
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC053_PAIR=0 "$p/main.cpp" -o "$d/base"
g++ -std=c++23 -O3 -DAHC053_PAIR=0 -DAHC053_MATCH=1 "$p/main.cpp" -o "$d/match"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/pair"
python "$p/benchmark_pairs.py" /path/to/gen /path/to/tester \
  "$d/base" "$d/match" "$d/pair" "$d/results.csv"
```
