# Hard-anchored service network: reach-constrained cut generation

Follow-up to the hard-anchor dev30 audit. A wall vertex is now cuttable only
if at least one presently legal adjacent stance, excluding the target's
forced open neighborhood, is reachable from an existing human in <=6 moves
on the current passable graph. This necessary condition is applied before
min-cut, not as a rejection of the resulting mask. Actual staffing still
requires exact distinct-worker matching within the projected retained
exterior. Same objective, anchors, <=7 walls, <=48 pen cells, same actual
release/+8/+16 snapshots. Previously routed nonrectangular masks are not
reranked. Nothing changes the solver's actions.

All 30 official scores, complete action hashes and independently replayed
scores matched the frozen narrow dev30. On 500 unchanged off-anchor animal
events: 476 no eligible <=7-wall cut, 21 same strict wall mask, one prior
already-routed known catch, and two new compact masks. One new irregular
13-cell seven-wall candidate (0000 t176) and one straight four-cell two-wall
candidate (0025 t176) both failed matching in the actual projected retained
side. Both subjects were normally caught. Zero new routable compact cuts and
zero novel otherwise-final-missed animals. No solver arm promoted.

This tests a necessary per-wall filter then the cheapest surviving cut, not
the exhaustive set of jointly staffed cuts or future animal trajectories.
The current-graph paths can differ from the eventual retained-side paths.
Counts are distinct observed events unless explicitly stated; repeat
snapshots can contain the same animal. Audit only; no score timing measured.

Run from this directory with the harness/tester and frozen narrow dev30
solver/input/control from the previous hard-anchor experiment:

```sh
python parallel_banks_anchored_reach_audit.py HARNESS TESTER NARROW IN \
  NARROW_CONTROL.csv parallel-banks-anchored-cut-dev30.jsonl.gz out.jsonl summary.json
```
