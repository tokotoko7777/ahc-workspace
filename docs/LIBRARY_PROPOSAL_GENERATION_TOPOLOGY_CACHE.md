# Library proposal: reuse component topology by board generation

**Status:** Used and verified in AHC008; standalone proposal, not a
shared API or a main-branch library replacement.

**When it helps:** One wall board is held across multiple interactive
turns, while moving entities repeatedly change the objective. Many
candidate cell-blocker sets recur. AHC008's late separator tests met this
pattern: both 100-case cohorts reused more than 63% of queried topologies.

**Practice:** Cache disjoint connected bit masks and areas, including a
sentinel for a cut that doesn't separate, indexed by the *exact* shape key.
Recompute counts of pets, workers, scores and feasibility with current
observations. Invalidate on board or connected-root change. If candidates
are not canonically indexed, avoid treating a hash match as equality. An
epoch-stamped index table and contiguous arena keep the hot path cheap.
For a 30×30 fixed board, `BitGridComponents<S,S>::Rows` is adequate.
For a new problem with frequent walls or one-shot candidate shapes, this
cache can be slower than direct flooding; measure hit rate before adding it.

```cpp
if (actual_walls != previous_walls || root != previous_root) clear();
const auto components = find_or_measure(candidate_shape_key);
// Always recompute dynamic information from the true current board:
for (const auto& component : components)
    evaluate(component.cells, component.area, latest_entities);
```

**Verified example:** `problems/ahc008/experiments/20261122/parallel-banks-narrow-seam-cache.cpp`
and adjacent notes, exact action/score ledgers and replay. Across two
100-case sets: same transcript per case, median paired tester time down
10.21% and 10.68%. Suggested shared helper only after a second use case;
a unit test should compare cached and uncached components while
interleaving entity moves, wall changes, root changes and duplicate queries.
