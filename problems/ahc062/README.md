# AHC062 — King's Tour

[Official problem and scoring](https://atcoder.jp/contests/ahc062/tasks/ahc062_a) ·
[Official generator and visualizer](https://img.atcoder.jp/ahc062/u5OpcTjC.zip) ·
[Official ranked result](https://atcoder.jp/contests/ahc062/results/json).

On a 200×200 grid, visit each cell once with four orthogonal or four
diagonal moves. The per-case score is `round(sum(k*A[path[k]])/40000)`
for zero-based visit days. Final is the sum across the 100 original cases,
larger better; no post-contest system rerun. The public result names
**gazelle** first without a score. Official direct case/score CSVs return
403, standings requires sign-in, and the potential rank-only archive
returned 404. The exact historical condition is **unjudged**.

## Weighted path updates

Initialize both arms with the same best of eight mirrored or transposed
snake paths. In each of exactly 280,000 planned proposals, the control
uses local-interior 2-opt: if the two new endpoint edges exist, reverse
the contiguous interval between them. The variant spends one proposal
in eight on a legal one-ended path rotation. Connecting the old endpoint
to its adjacent interior vertex permits reversing a prefix or suffix
without forming an invalid edge.

The route requires finding vertices by stable board index even after
large interval reversals, and evaluating a weighted position sum. The
existing segment tree uses fixed positions and was not suited for both.
The new [weighted sequence treap](../../library/weighted-sequence-treap.hpp)
stores count, sum and local indexed moment per subtree. The reversal
delta for interval length L is `(L-1)*sum - 2*moment`, with no dependency
on its global offset. Reverse lazily; item rank and indexed lookup are
expected log-time. Test: 28,800 random accept/reject reversed intervals
against plain vector replay, including negative weights, rank and lookup.

**Next hypothesis:** construct two complementary lanes within each
paired row band. Choose low-weight cells into the early lane per column
and high-weight cells into the late lane. Across each neighboring band
pair, use two reserved border columns as a 4×2 connector and mirror it
on alternating sides. One connector lane goes through the inner top
cell, all four outer cells, and the inner bottom cell; the other uses
the two remaining inner cells. This accepts either port ordering in
each band because the two ports are only one row apart. Join at the
free 2×2 ends, then spend the same exact local-search proposal budget.

| Fresh cases | 2-opt score ↑ | Endpoint rotation score ↑ | Win / tie / loss |
|---|---:|---:|---:|
| Dev (18) | 405,460,966 | 405,579,904 | 8 / 0 / 10 |
| Holdout (18) | 405,684,739 | 405,405,613 | 10 / 0 / 8 |

Holdout median gain 38,299 but mean loss 279,126; keep endpoint rotations
optional and use the interior 2-opt as default. Both checked exactly
280,000 proposals per case. Mean initial holdout snake score was about
400.807M; 2-opt moved that to 405.685M. All 72 runs agreed with the
official visualizer and independent exact integer path/score replay.
Maximum solver wall time 0.445166 s of 3 s. [Logs](benchmark-results.csv).

```sh
p=problems/ahc062
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC062_LANES=0 "$p/main.cpp" -o "$d/twoopt"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 -DAHC062_LANES=0 -DAHC062_ROTATE=1 "$d/submission.cpp" -o "$d/rotation"
python "$p/benchmark.py" /path/to/gen /path/to/vis \
  "$d/twoopt" "$d/rotation" "$d/results.csv"
g++ -std=c++23 -O3 tests/weighted_sequence_treap_test.cpp -o "$d/treap-test"
"$d/treap-test"
```

## Follow-up: complementary early/late lanes

In each paired interior strip, move one column at a time. The early path
uses the lower-valued cell in each column, and the late path uses its
complement. Each two-cell pair's next column is adjacent to either choice
under king adjacency. Reserve two border columns on both sides. Between
two neighboring paired strips, on the side where the paths turn, use
a disjoint 4×2 gadget. One route takes the near top cell, all four far
cells vertically, and the near bottom cell; the other takes the remaining
two middle near cells. Both entry and exit options in the adjacent
interior column are within one row of either routed lane's assigned port.
Alternating sides makes the gadgets disjoint. The only unclaimed
rectangles are the top and bottom 2×2 hooks; follow early lane, bottom
hook, reversed late lane, then top hook. Thus each interior cell belongs
to exactly one strand, and each border cell belongs to one connector/hook.
Mirroring or transposing preserves coverage and every step. All 16
connector/orientation possibilities are tested for six even board sizes,
8 generated weights each, including the contest size (768 exact paths).

The same 280,000 interior 2-opt proposals polish either starting family;
the snake remains a legal fallback. No finals used.

| Fresh cases | Snake + 2-opt ↑ | Two lanes + 2-opt ↑ | Better / tie / worse |
|---|---:|---:|---:|
| Dev (18) | 405,704,533 | 467,302,960 | 18 / 0 / 0 |
| Holdout (18) | 405,801,993 | 467,170,089 | 18 / 0 / 0 |

Holdout paired median gain 61,601,892. All 72 official and independent
evaluations agreed; all 280,000 proposals completed. Max solver wall
0.698 s. Keep lanes as branch default. [Data](benchmark-lane-results.csv).
Next attempt four ordered weight bands instead of two, but first prove
a four-route disjoint border cover with arbitrary interior ports or
explicitly constrain the selectable quartet permutations. Do not infer
historical rank from different seeds.

```sh
p=problems/ahc062
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC062_LANES=0 "$p/main.cpp" -o "$d/snake"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/lanes"
python "$p/benchmark_lanes.py" /path/to/gen /path/to/vis \
  "$d/snake" "$d/lanes" "$d/results.csv"
g++ -std=c++23 -O3 tests/ahc062_ladder_test.cpp -o "$d/ladder-test"
"$d/ladder-test"
```
