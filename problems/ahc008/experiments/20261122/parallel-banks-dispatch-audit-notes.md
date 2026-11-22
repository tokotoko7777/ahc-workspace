# Read-only fallback ordinary wall-dispatch matching audit

The narrow rescue ordinary `pocketPlan` accepts all unblocked immediate
builder/wall edges before any positive-ETA walk edge: the former has
key 100, and the latter at most 94. The suspected arrival-before-build
priority inversion does not occur under this score. A greedy maximal
matching can still leave a feasible exact-edge augmenting path, so
compare it to maximum bipartite cardinality.

Use the same already-enumerated worker/exterior stance pairs, preserving
earlier standard reservations/moves and the current last-wall
evacuation restriction. For each immediate matching improvement,
look for an actual local build assignment increasing immediate walls
while keeping all humans in each original component connected, after
other already accepted same-turn walls. This is a local feasibility
certificate, not a subsequent behavior prediction. With the same
greedy immediate builds pinned, compare maximum versus actual
positive-ETA reachable worker/job assignment cardinality (not
globally minimum route cost or many-turn assignment persistence).
No action changes; all 30 full hashes and scores match and pass official
v3 recorded playback and independent replay.

Across **1,899** posthandoff ordinary pocket dispatch calls, maximum
matching found only **3** additional immediately legal wall builds,
in three distinct turns, two distinct target episodes and two cases.
All three have an exact retaining-connectivity witness. Fixing immediate
builds to the existing greedy choices, the matching never increases
travel assignment cardinality (0/1899). No credible volume from this
single-turn matching path; no live A/B, held-out cases, or promotion.
This does not address proper time-to-evacuate in wide-cut planning.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-dispatch-audit.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-dispatch-audit.patch"
g++ -std=c++23 /tmp/ahc008-dispatch-audit.cpp -o /tmp/ahc008-dispatch-audit
python "$D/parallel_banks_dispatch_audit.py" "$HARNESS" "$TESTER" /tmp/ahc008-dispatch-audit "$DEV30" "$D/parallel-banks-empty-guard-dev30.csv" /tmp/dispatch.csv /tmp/dispatch.json
```
