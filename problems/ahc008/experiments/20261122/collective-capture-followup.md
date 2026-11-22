# Collective capture follow-up: baffle annex and shared-stance hub

Both diagnostics use the frozen capture4 standalone on the same 30 generated
development inputs as the preceding collective cut audit. Input hashes/seed
recipe and original capture baselines are in the adjacent capture records.
Each driver runs official AHC008 v3 tester, then plays back identical action
bytes and requires agreement with independent replay. This is not a new solver
benchmark or evidence from official distributed practice cases.

## Annex a clean half just before evacuating

At T−1 before the first durable all-clear, enumerate straight baffles across
each adjacent occupied macro room. Require the clean part to be connected to
the original safe union through a separate portal, preserve all the original
safe cells, and recount the allowed walls and exact connected human union.
A single baffle promises 20.0 extra safe cells on average, max 36, with any
gain on 22/30. Greedily combining compatible baffles raises the mean to 21.0
and the maximum to 66, the only case ≥60. None can be done with at most two
added walls, nor by instant or ≤2-turn stance matching. Do not promote unprepared
T−1 annexes. This does not rule out early prebuilt baffles that change the
actual animal distribution and wall costs.

## T-junction guard: separate arms share one stance

Enumerate a guard position near a macro doorway, three neighboring gate cells,
a fourth open approach, and three disjoint arms of three cells each. Each arm
has geometrically closure-safe poses of both checkerboard colors. Retain two
geometries per stance/gate triplet and cap extra walls at 15 relative to a
finished macro skeleton (13,026 eligible geometries before sampling, 628 after).
Keep actual existing required macro support. The added arm walls are hypothetical
in the main experiment: replayed pet trajectories are **not** recomputed on a
modified board. Consider complete windows starting every five turns from 35,
and ending by first durable all-clear or turn 220. Maximize retrospectively over
both the layout and epoch. Each turn can close at most one gate, each arm can
close at most once, and the same pet ID may count only once.

| Retrospective measure | 10 turns | 20 turns | 40 turns |
|---|---:|---:|---:|
| Best distinct pets, mean over 30 | 3.37 | 3.57 | 3.87 |
| Also actual person within 2 steps at epoch start | 2.77 | 3.07 | 3.53 |
| Best, staff-near, max case | 4 | 4 | 5 |

A nearby human is not reserved or charged for leaving their macro job; neither
construction nor changed stochastic trajectories nor a causal selection
rule is modeled. Values are an optimistic selection statistic on these
unmodified histories, not a formal upper bound for a different policy.
The count of eligible windows counts duplicated geometry/arm opportunities,
not captures. The actual-wall ablation requires every wall around all three
arms to already exist; it found no eligible windows on all 30 traces. See each CSV. Do not adopt this as an additive fifteen
wall project. A separate next feasibility gate would compare one permanently
staffed guard vs none in an integrated prebuilt layout with official animal
transitions, on causal fixed geometry and including no-arrival cases and construction
cost.

Reproduce from repo root (HARNESS contains the archived v3 replay and
benchmark Python scripts; CASES is the same collective cut dev30 input dir):

```bash
g++ -std=c++23 problems/ahc008/capture_first_submission.cpp -o cap4
d=problems/ahc008/experiments/20261122
python "$d/collective_baffle_audit.py" "$HARNESS" "$TESTER" ./cap4 "$CASES"
python "$d/guard_hub_audit.py" "$HARNESS" "$TESTER" ./cap4 "$CASES"
python "$d/guard_hub_audit.py" "$HARNESS" "$TESTER" ./cap4 "$CASES" --actual-only
```
