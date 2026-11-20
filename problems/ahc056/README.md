# AHC056 — Grid Turing Robot

- [Official problem / scoring](https://atcoder.jp/contests/ahc056/tasks/ahc056_a)
- [Official generator / visualizer](https://img.atcoder.jp/ahc056/zUbWUSnS.zip)
- [Official result metadata](https://atcoder.jp/contests/ahc056/results/json)

A program visiting every destination within `T` receives raw cost `C+Q`;
incomplete programs receive the larger penalty `2N^4+(K-V)N^2`. Lower raw
cost is better. Across 2000 final cases, the sum of per-case rounded
`10^9 * minValidRaw / ownRaw` is the relative score, higher better.
The official [input metadata](https://img.atcoder.jp/ahc_standings/ahc056/input.csv)
SHA-256 is `b28ae9fe0cc4042e2c8ab6de33107e238d956fd58f803c979d49f526604886c1`
(fields file/seed/N/K/T/X); the official
[score matrix](https://img.atcoder.jp/ahc_standings/ahc056/result.csv)
SHA-256 is `1d27cbb3f26e08ec3ec8f81c2c63dae7bc7ff0a2b66945d3412591a3185c0116`.
Recomputing case minima across the 1105 public rows yields historical
first-place **saharan, 1,899,994,171,242**, matching the public rank metadata.
The official [final seed list](https://img.atcoder.jp/ahc056/seeds.txt) has
published SHA-256 `c7a6f9526da67d83d1f8fed006e1b8f5edc8e4134444cf954e400a4dcd8bd130`.
No official historical cases have been run by this solver: **unjudged**.
The winning raw mean is 28.097 on historical cases, not directly comparable
with the fresh benchmark mean below.

## Backward transition compilation

Both arms use the same concatenated shortest route between all consecutive
targets via the repository's [Graph BFS](../../library/graph-bfs.hpp).
Working backwards, remember the required color of each future cell visit
and the required state on the next chronological step. For a step at a
given cell, its rule signature is the already known (repaint color, next
state, move). Assign a current (color, state) to this signature. Preprocessing
the previous visit to that cell later in the backwards scan will set exactly
this color; the previous time step will set exactly this state. Therefore
the planned action comes out by forward induction. A consistent state
renumbering makes the initial state zero.

Control chooses the state from a precomputed time block or a modular cycle,
and interns signatures per state. The candidate removes the time constraint
and reuses any existing rule of the same signature, even in a different
state. For new rules, distribute their colors across states to balance
maximum palette size, with a second strategy preserving the next state
when feasible at nearly equal load. The solution keeps the best complete
cost including the control. At most one rule per planned step is emitted.

| Fresh cases | Scheduled raw ↓ | Adaptive raw ↓ | Better / Tie / Worse |
|---|---:|---:|---:|
| Development (32) | 77.8125 | 70.90625 | 32 / 0 / 0 |
| Holdout (32) | 80.5000 | 73.65625 | 32 / 0 / 0 |

Holdout median saving 7 colors/states. All 128 runs reproduced every target,
the exact same step count per pair, all size and uniqueness conditions and
the official score in an independent simulator. Max measured solver wall
0.455221 s out of 2 s. Adaptive allocation is branch default.
[Paired logs](benchmark-results.csv).

The existing graph BFS fits the route construction directly. The new
behavior is problem-specific reverse instruction synthesis (a per-cell
mutable symbol plus one global state), so no new library component was
introduced in this slice.

**Next hypothesis:** do not treat the BFS tie-breaking as incidental.
Choose same-length routes whose runs and transitions can share an
instruction pattern: favor reuse of the same departure direction at a
cell and long straight segments as distinct controls, then compile the
same trajectory budget with the frozen backend. Also test symbol/state
joint compaction while verifying the exact original trace.

```sh
p=problems/ahc056
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC056_ALLOCATE=0 "$p/main.cpp" -o "$d/base"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/allocate"
python "$p/benchmark.py" /path/to/gen /path/to/vis \
  "$d/base" "$d/allocate" "$d/results.csv"
```
