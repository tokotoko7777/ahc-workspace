# Fixed-topology CSR Dinic (library candidate)

A build-once C++23 directed max-flow graph. Supply all arcs to
`ahc::CsrDinic(vertices, arcs)`, call `solve(source, sink)` once, then use
`on_source_side(vertex)` to query the canonical minimum cut. See
[`example.cpp`](example.cpp) for a four-vertex executable.

- Vertex IDs are integers in `[0, vertices)`; source and sink must differ.
  Endpoints are checked even for discarded zero-capacity arcs and loops.
- Capacities are nonnegative `uint64_t` values. For signed inputs, validate
  nonnegativity before converting them. At solve entry the implementation
  checks that the sum of nonloop outgoing source capacities fits in
  `uint64_t`. This conservative requirement can reject graphs whose
  actual maximum flow would fit anyway. A rejected solve leaves the graph
  unchanged. Residual capacity in an original arc's forward/reverse
  pair sums to its original capacity, so reverse updates cannot overflow.
- Parallel/antiparallel arcs are distinct; zero capacities and self loops
  do not affect flow or cuts and consume no residual slots. Construction
  copies arc data into CSR; the input may be discarded afterwards.
- No partial flows, changing capacities, graph copying, or solver reset.
  Recursive current-arc DFS uses stack proportional to level-path length.
  Prefer an iterative library for graphs with very long paths or a small
  stack limit.

## Why the residual side is canonical

On successful completion, the final failed level BFS records all
vertices reachable from the source by positive residual arcs. That set
is itself a minimum source side by the max-flow/min-cut theorem. Every
minimum source side must also contain every residual-reachable vertex:
otherwise it would have a positive residual arc crossing out, contrary
to saturation across a minimum cut. The final set is therefore the
intersection of all minimum source sides, independent of arc order or
which maximum flow was found. For each graph, the oracle in
[`test.cpp`](test.cpp) enumerates every source/sink-respecting cut and
checks both minimum capacity and that intersection.

## When to adopt

Use as a separate static-graph alternative, not a replacement for a
mutable max-flow builder. Build materializes two residual records per
nonzero nonloop input arc, plus four vertex arrays (one has an extra
slot). CSR avoids per-vertex edge allocations and puts each traversed
adjacency in one contiguous slice. Original arcs are still held by the
caller while constructing, and wide capacities/indexes can use more
memory per edge than a tuned 32-bit solver. On a typical 64-bit ABI the
residual records are 24 bytes each (48 bytes per retained input arc),
plus approximately 24 bytes per vertex, excluding allocator overhead
and the input arc list. No generic speed claim is made without a matched
benchmark.

This implementation is independently adapted from the team's verified
[experimental Chase CSR kernel](../RESULT.md) (contiguous
adjacency, paired reverse indices and reuse of final failed BFS); it
does not use contestant reference code or AHC008-specific pruning or
policy. The Chase integration preserved complete actions and score at
fixed work and lowered whole-panel CPU about 25–26%. The faster native
clock evaluated more terminal rollouts and failed the distributed score
gate, so it was **not** adopted as the default AHC008 solver. That policy
result is distinct from algorithm correctness and the case for an
optional library component.

Run the example and the exhaustive small-graph/boundary test:

```sh
g++ -std=c++23 -O2 -Wall -Wextra -pedantic test.cpp -o /tmp/csr_dinic_test
/tmp/csr_dinic_test
g++ -std=c++23 -O2 -Wall -Wextra -pedantic example.cpp -o /tmp/csr_dinic_example
/tmp/csr_dinic_example
```
