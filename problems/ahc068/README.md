# AHC068 — Rectangle Swap

[Official rules](https://atcoder.jp/contests/ahc068/tasks/ahc068_a) ·
[Official generator and visualizer](https://img.atcoder.jp/ahc068/FCpa2O61.zip) ·
[Official ranked result](https://atcoder.jp/contests/ahc068/results/json).

A fully sorted 20×20 board scores
`400 + round(10^6 * log2(100000 / operation_count))`; smaller iteration
counts give larger scores. Otherwise the score is the number of correctly
placed cards. Sum scores over 150 original contest cases, with no system
rerun. Official results name **PrussianBlue** as winner, but do not expose
the score. Official case/score CSVs returned 403 and rank-only fallback
404. Historical numeric score / exact case comparison: **unjudged**.
No contest cases or participant solutions were accessed.

## Route target cards in the still-active graph

Both arms use the same reverse BFS discovery order for fixing cells. The
parents along the original connected spanning tree remain active until
after their descendants, hence the active subgraph never disconnects.
Find the target card using its inverse position and BFS on allowed moves.
When it is placed, freeze that cell; future moves never touch it.

The baseline only swaps one pair of adjacent cells. The candidate can
also move a card by distance `d` along a wall-free active row or column
run when a contiguous interval of length `2d` can swap it into the desired
slot. Each such strip is one legal operation. For multiple possible strip
starts, select the greatest exact change in total Manhattan displacement
on touched cards. Strip supports may contain other cards, still active
and not required for the current target. They are all updated in the
inverse position map. Both fully sort; choose fewer actual moves.

| Fresh cases | Adjacent moves ↓ | Strip moves ↓ | Strip score ↑ |
|---|---:|---:|---:|
| Dev (26) | 6,298.846 | 1,550.808 | 6,070,691.27 |
| Holdout (26) | 6,243.577 | 1,528.731 | 6,100,010.58 |

Strips beat the adjacent control in all 52. Holdout median savings 4,842.5
moves. All 104 boards fully sort and independently match every required
empty wall and exact official score. Max observed .046352 s of 2 s.
Keep strips as default. [Logs](benchmark-results.csv).

Picking a strip start on a blocked/truncated line requires more than
knowing the card displacement. Added
[`half_swap_interval_starts`](../../library/half-swap-interval.hpp)
to compute the feasible contiguous start range directly; tested against
enumeration for 34,110 small range/source/destination combinations.

**Next hypothesis:** widen each chosen strip in the perpendicular
direction, retaining its exact target-card motion. Check all included
cells active and all inside walls absent. At equal route-generation
work, choose the broader rectangle only when collateral cards' wall-aware
distance improves; compare to extra same-cost one-strip offset choices
and verify that no frozen cell is moved.

```sh
p=problems/ahc068
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC068_WIDEN=0 -DAHC068_STRIPS=0 "$p/main.cpp" -o "$d/base"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 -DAHC068_WIDEN=0 "$d/submission.cpp" -o "$d/strips"
python "$p/benchmark.py" /path/to/gen /path/to/vis "$d/base" "$d/strips" "$d/results.csv"
g++ -std=c++23 -O3 tests/half_swap_interval_test.cpp -o "$d/test"
"$d/test"
```

## Follow-up: perpendicular expansion, equal shape evaluation

The inherited route is always retained as fallback. Both additional arms
use the same active half-strip routing and wall-aware shortest-path
distances. For each strip start, they evaluate the same adjacent strips
up to three away in each perpendicular direction. Every included strip
must be fully active and have no along/across internal wall. Choose the
best positive cumulative collateral gain separately on both sides, and
add this to the priority of a strip's starting offset. The narrow control
uses this extra computation to choose the same best offset, but executes
only its main strip. The widen arm executes the full legal rectangle,
keeping the target card's exact planned swap. This isolates collateral
execution instead of comparing against a cheaper offset search.

| Fresh cases | Frozen ops ↓ | Narrow score-only ops ↓ | Wide executed ops ↓ |
|---|---:|---:|---:|
| Dev (26) | 1804.269 | 1786.154 | 1606.115 |
| Holdout (26) | 1641.077 | 1626.269 | 1485.500 |

Wide beats both on all 52 cases; holdout median gain 129 vs the narrow
control. Mean held-out collateral probes 4061 wide vs 4640 control; they
diverge after swaps and wide finishes earlier, but cap and per-offset
legality/potential computations match. Medians .00765 s and .00791 s.
All 156 solutions sorted fully with exact official scoring and independent
wall and permutation replay. Max wall .015539 s; widening is default.
[Logs](benchmark-rectangle-results.csv).

Next use a short global preprocessing sweep of net-beneficial rectangle
swaps before fixing any cell, then compare against an equal-work sweep with
size-one orthogonal dimensions only. Protect a full legal fallback.

```sh
p=problems/ahc068
d=$(mktemp -d)
for i in 0 1 2; do
  g++ -std=c++23 -O3 -DAHC068_WIDEN=$i "$p/main.cpp" -o "$d/arm$i"
done
python "$p/benchmark_rectangles.py" /path/to/gen /path/to/vis \
  "$d/arm0" "$d/arm1" "$d/arm2" "$d/results.csv"
```
