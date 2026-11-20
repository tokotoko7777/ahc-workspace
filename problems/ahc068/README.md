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
g++ -std=c++23 -O3 -DAHC068_STRIPS=0 "$p/main.cpp" -o "$d/base"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/strips"
python "$p/benchmark.py" /path/to/gen /path/to/vis "$d/base" "$d/strips" "$d/results.csv"
g++ -std=c++23 -O3 tests/half_swap_interval_test.cpp -o "$d/test"
"$d/test"
```
