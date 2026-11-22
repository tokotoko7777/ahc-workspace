# Passable-graph ordinary min-cut radius (dev30; not adopted)

Change one dimension of ordinary post-bank rescue: use actual four-neighbor
shortest-path distance in the observed passable grid both for the protected
pet footprint (`d < rr`) and the bounded local network (`d <= rr+flowSpare`).
The border at the latter distance connects to the sink; because path distance
changes by at most one along a passable edge, a flood cannot leave the local
network without passing a border. Cells are enumerated in the old order;
the original Manhattan bounding box is still a safe enumeration superset.
One bounded BFS histogram is used across all radii for a center and the
optimistic pruning penalty subtracts the *actual* protected core size.
No changes to evaluation, candidate pet ranking, builders, legality, gates,
bank dynamics, rescue switch, or terminal. Flags `5 1 0` are Manhattan
(control), `5 1 1` graph. No-argument behavior is unchanged.

Motivation: the previously built bank scaffold puts cells on the other side
of a brace row in a Manhattan core even when the pet requires a significant
detour to reach them, making an unnecessarily large seed. Actual routes
could make smaller irregular pockets while respecting the legal guard
geometry. The high-level min-cut idea relates to the user-authorized
[TERRY](https://blog.terry-u16.net/entry/ahc008) and irregular-shape motivation
to [simanman](https://simanman.hatenablog.com/entry/2022/02/28/074838); no
submission code was accessed.

| Old dev30, official v3 and exact replay | Manhattan | Graph |
|---|---:|---:|
| Score | 697433161 | 696595148 |
| All clear | 14 | 12 |
| Residual reachable animals | 40 | 55 |
| Distinct safe area sum | 5026 | 4911 |
| Paired bank exact-four animals | 199 | 199 |
| Actual 4–48 cell captures after release | 53 | 48 |
| Actual nonrectangular 4–48 cell captures after release | 48 | 45 |
| Flow calls | 1219 | 1207 |
| Total input flow vertices (cells before splitting) | 202870 | 190550 |
| Feasible flow cuts | 1200 | 1191 |
| Flow candidates chosen by selector | 47 | 53 |
| Median/max tester sec | .362/.471 | .358/.539 |

All 60 runs were legal and exact replay reconciled; the two arms shared
every action byte before the normal release (160/225 per case), and the
control matched all 30 frozen rescue hashes. W/T/L 6/17/7, full outputs same
in 16/30 cases. Shape counts are exact pre-pet-response first separations,
and include all fallback actions rather than attributing every capture
to min-cut generation. One example of a new real separation is dev 0005,
zero-based t297, a cow in a 35-cell nonrectangle. This is not a net benefit.
The graph change produced smaller aggregate flow networks but fewer small
captures and fewer cleared people (113→106); more chosen cut candidates did not produce
better plans. No untouched or official practice100 used. Do not promote.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-graph-flow.cpp "$D/parallel-banks-rescue-research.cpp" "$D/parallel-banks-graph-flow.patch"
g++ -std=c++23 /tmp/ahc008-graph-flow.cpp -o /tmp/ahc008-graph-flow
python "$D/parallel_banks_graph_flow_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-graph-flow "$DEV30" /tmp/graph.csv --frozen-control-csv "$D/parallel-banks-rescue-dev30.csv"
```
