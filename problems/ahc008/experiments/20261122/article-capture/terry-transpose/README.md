# Faithful orientation: fixed prelude-only screen

No forecast, layouts, species weights or capture logic changed. For
M>=7 and stock NN map 2/3 only, compare stock and transposed starts relative
to the very same original terrain. Stably sort original aisle columns by
abs(col−15), then make M entry slots by repeatedly cycling that ordering
(duplicate columns remain different crew slots). For each true initial
worker BFS on the nominal fully built X-only board (doors open), allowing
the initialized start to exit even when planned X, like original Nav.
Distance to a slot is minimum over all open cells on its column. An exact
bitmask DP assigns a distinct worker to every one of the M slots and
minimizes maximum distance. Use transpose only if stock ETA minus
transpose ETA is >=4; threshold fixed before any generation. This is a
geometry proxy, not predicted actual completion (no worker queue jobs,
pet blockers, role, or post-setup path effects).

Transpose is the involution (row,col) -> (col,row), U↔L and D↔R and
corresponding lower-case walls, dot unchanged; apply to starts, all
incoming pet steps and outgoing worker actions. Do not change original
internal tie ordering or map IDs. Physical equivalence of representation
does not imply equal score between orientations because starts differ.

The dedicated namespace is ahc008-terry-transpose-eta4-v1. Commit both
4096-seed lists before generation; first 8 bytes of SHA256 namespace,
panel and consecutive index in little-endian order. Analyze only
prelude, no private generator suffix except hash for case identity.
Run `python ahc008_terry_transpose/screen.py ahc008_terry_transpose/dev --generator ahc008_focus_assets/tools/target/release/gen`.
Pilot first 30 qualifying in stream order; reserve first100 in a separate,
still unopened fresh-confirm namespace if pilot supports it. No selection
by result, no solver or scoring run by this screen.

Dev geometry: 4096 total, 2693 M>=7, 2613 stock map2/3, 466 eligible
(11.38% all seeds, 17.83% in stock map23/M>=7). Split map2 184, map3 282.
Dev generated exactly once; first30 inputs are copied to dev/selected and
manifest records input hashes and exact ETAs. Confirmation contains the
precommitted seeds only. The ETag of each seedlist is the SHA256 printed
by the prelude manifest for dev or listed below for confirmation:
889766f2b23e0bf4f796a2eb62141148f589a02854b83747ae3611fd1aea1d06
6a5ba004b6eda83855fe994498261e0f88e7adfa76ccd562aa1325b6303140de

## Official pilot rejected

Standalone GNU C++23 source SHA
`6016efb8ae1414f06a56d0a41843c9e261bf1f51395e51703ff870875329fa1e`.
The optional flag implementation passed exact transpose involution on four
frozen 300-turn tapes (one per observed map ID 1–4, zero terminal pairs);
no flags exactly matched each original command stream and mapping both
observations and output produced the exact transposed stream. This is a
coordinate/protocol unit, not evidence that interchange improves score.

All 60 serial official runs and independent replay/scoring on the fresh
prelude-qualified 30 are legal. CSV arm `v25` denotes the no-option
faster faithful executable (not old v25); arm `terry_independent` adds
only `--transpose`.

| Metric | Original coordinates | Transpose |
|---|---:|---:|
| Score | 1,930,777,777 | 1,921,222,223 |
| All clear / useful ≥300 | 30 / 30 | 30 / 30 |
| Mean final human area | 579.23 | 576.37 |
| Median nominal construction rounds | 58.5 | 58.5 |
| Median first forecast assignment | 63 | 62 |
| Maximum local tester | 2.738 s | 2.721 s |

10 better scores, one tie, 19 worse. Score gate fails despite the dry aisle
ETA advantage and full-capture tie. No held evaluation, no further orientation
thresholds, and no change to the selected faster exact-coordinate policy.
The source/evidence archive excludes the user-supplied model data and any
generated standalone carrying it.
