# AHC064 — Non-Crossing Railcar Rearrangement

[Official problem / scoring](https://atcoder.jp/contests/ahc064/tasks/ahc064_a) ·
[Official generator / visualizer](https://img.atcoder.jp/ahc064/fLqO0Ras.zip) ·
[Official ranked result](https://atcoder.jp/contests/ahc064/results/json).

A completely sorted yard scores `5000 - turns` per case, larger better;
partial solutions use correct-track and correct-position bonuses instead.
The final contest sums 150 original cases with no separate system rerun.
Official results JSON identifies **chokudai** as rank 1 without score.
Direct score/case CSVs return 403 and rank-only ALE fallback 404. Numeric
historical score and same-case conditions are unverified: **unjudged**.

## Same operation trace, shorter parallel schedule

All arms start with a legal serial operation list. Empty each departure
track into its corresponding siding in one parallel turn. Read target
cars in sorted order, temporarily moving blockers onto other departures
with spare capacity, extract the next consecutive correct block, and
return blockers in reverse operation order. This always restores their
relative order. A completed departure has at least five free spaces so
the at-most-nine blockers fit using two or more other departures.

Build a dependency DAG with an edge from each operation's last user of
each of its two tracks. Any topological execution preserves the order of
operations on each track, which determines that track's contents and
capacity. Simultaneous turns are chosen from ready nodes with strictly
increasing departure and siding indexes. This also enforces endpoint
uniqueness. One packer maximizes number of operations per turn; the other
maximizes sum of squared remaining critical-chain lengths, then retains
the faster schedule of the two.

| Fresh cases | Serial turns ↓ | Cardinality ↓ | Critical portfolio ↓ |
|---|---:|---:|---:|
| Dev (30) | 239.933 | 134.700 | 126.733 |
| Holdout (30) | 241.933 | 136.333 | 128.833 |

Critical portfolio beat cardinality in all 30 holdout cases, by median
7 turns. The complete operation count and the per-track operation traces
were identical for all three arms on all cases. All 180 outputs finished
the full sorting and agreed with official visualizer and independent
legality/score replay. Maximum measured solver wall 0.038199 s of 2 s.
Keep critical as default. [Results](benchmark-results.csv).

The existing LIS targets cardinality, while this problem requires maximizing
priority weight with strict increases on *both* endpoints. Added
[`maximum_weight_noncrossing_matching`](../../library/noncrossing-matching.hpp)
with exhaustive-subset differential tests on 1,800 small cases, including
duplicate endpoints and negative weights.

**Next hypothesis:** the number and location of blockers moved out and back
depends strongly on extraction order. Keep the same exact simultaneous
scheduler but try interleaving the next requested cars across destination
departures based on blocker count, consecutive target run and next-track
load. Compare at equal construction count, using randomized feasible order
as an explicit control.

```sh
p=problems/ahc064
d=$(mktemp -d)
for i in 0 1 2; do
  g++ -std=c++23 -O3 -DAHC064_PACK=$i "$p/main.cpp" -o "$d/arm$i"
done
python "$p/benchmark.py" /path/to/gen /path/to/vis \
  "$d/arm0" "$d/arm1" "$d/arm2" "$d/results.csv"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/submission"
g++ -std=c++23 -O3 tests/noncrossing_matching_test.cpp -o "$d/match-test"
"$d/match-test"
```
