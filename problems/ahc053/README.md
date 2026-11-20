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
g++ -std=c++23 -O3 "$p/main.cpp" -o "$d/base"
g++ -std=c++23 -O3 -DAHC053_MATCH=1 "$p/main.cpp" -o "$d/match"
python "$p/benchmark.py" /path/to/gen /path/to/tester "$d/base" "$d/match" "$d/results.csv"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/submission"
g++ -std=c++23 -O3 tests/sorted_bottleneck_assignment_test.cpp -o "$d/test"
"$d/test"
```
