# Hard-anchored existing service roads, compact cut feasibility

Read-only on 30 frozen narrow-rescue development executions. All action
hashes, scores, official recorded playback and independent replay match.
Sample the actual handoff, +8, and +16 turns, without conditioning
on input species or t160 residual. Retain the current connected service
lane/crossbar skeleton, deterministically connect any hole-split patches
and each human through a real shortest passable path. Every protected
vertex is forced to the same source side of a whole-grid vertex cut;
one currently reachable off-anchor animal is the sink. Unlike prior
graph-radius or one-shot local proposals, the cut cannot sacrifice the
existing protected access paths or any human positions. This is an
independent adaptation of the protected roads concept in the
[user-authorized TERRY narrative](https://blog.terry-u16.net/entry/ahc008),
not an implementation borrowed from a submission.

The exact mincut prioritizes job count, then current physical stance
distance. One minimum cut per animal/sample and legality mode, not
exhaustive shape generation. Strict: no standing human, pet or orthogonal
pet-neighbor cell may be a target wall. Geometry-only alternative
relaxes the latter neighbor condition but records actual blockers.
Take <=7 absent wall jobs, exact flooded animal-only component <=48,
nonrectangular reported separately. Restore non-boundary far-side jobs
that do not alter the retained flood. From the projected retained
exterior, find one distinct worker per pending wall, each with actual
passable route within six steps to a stance. This is a deliberately
short same-time window; it neither assumes the pet will remain nor
rules out sequential multi-job crews or other minimum-cut ties.

90 snapshots all have connected anchors, with mean 169.3 protected cells;
678 reachable animal events, 500 off-anchor. Strict results:

| Gate | Events | Cases | Distinct final-unresolved individuals on ordinary tape |
|---|---:|---:|---:|
| Current legal compact <=48 | 95 | 23 | 11 (7 cases) |
| Also nonrectangular | 70 | 21 | 7 (5 cases) |
| Also distinct real route to stance <=6 | 1 | 1 | 0 |
| Already at all stances | 0 | 0 | 0 |

The 70 legal nonrectangular geometry events are typically staffing-heavy:
36 need seven distinct jobs, 28 need five, and only one or two events
each need two/three/four. Median actual pen area five, maximum 37.
Only routed case 0000 t168 has a four-cell nonrectangular cow pen
(0-based shape IDs 600/630/631/660), two legal gate jobs with one
step per worker, retained area 682. The cow was ordinarily caught
anyway; not a new or observed capture. Relaxed geometry yields
additional smaller nominal cuts but no further legal routed nonrect.
No live change, stronger staffing claim, cohort or adoption. Toy fixture
independently checks a single legal wall cutting a known L-shaped six
cells and immediate builder reach while preserving all anchor cells.

```sh
D=problems/ahc008/experiments/20261122
python "$D/parallel_banks_anchored_cut_audit.py" --fixture
python "$D/parallel_banks_anchored_cut_audit.py" "$HARNESS" "$TESTER" "$NARROW" "$DEV30" "$D/parallel-banks-empty-guard-dev30.csv" /tmp/anchor.jsonl /tmp/summary.json
```
Uses NumPy, SciPy and the audited exact-replay harness.
