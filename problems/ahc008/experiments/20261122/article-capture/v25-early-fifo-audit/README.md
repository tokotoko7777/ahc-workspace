Generate from the pinned baseline (from workspace):
    python ahc008_v25_fifo_audit/make.py ahc008_v24_standby_early_seam/record/solve-v25.cpp ahc008_v25_fifo_audit/solve.cpp
    g++ -std=c++23 ahc008_v25_fifo_audit/solve.cpp -o ahc008_v25_fifo_audit/solve
    python ahc008_v25_fifo_audit/summarize.py RAW/*.log

Run with unique AHC008_FIFO_AUDIT=RAW/CASE.log on each frozen M5-dev30 input. Check historical action hashes, scores and legality independently; report tape divergence rather than silently substituting it.

W: t, worker, current stance, next distinct stance or -1; then length-prefixed lists of pending queue head walls, members actually pet-blocked, legal same-stance later-owned walls, and legal walls at the next distinct scheduled pending stance if actually adjacent. Checks current move/build reservations plus original intended blocks. The five-board launch gate credits same-stance legal targets only when all pending head walls are currently pet-blocked, never adjacent-next or nonpet conflicts.

T: t, tube, number of real axial pairs, number unreserved, four worker bitmasks for blocked-dot/walk/build/other; then length-prefixed missing shoulders and safe current animal IDs. Ready tubes with service-bridge conflict are excluded; missing-shoulder rows are not simulated closures, and unavailable exact pair does not imply inability to arrive; no future ETA forecast. Both sampled post-builder/pre-hunter. No program action selection changes. Logging may affect clock-dependent old decisions, which is why action digest checks matter.

## Verified result

All 30 exact v25 action hashes and official/independent scores reproduced.
716 worker-turns blocked by pets at the FIFO head. No legal work at the
same stance, hence 0/30 pass the five-board launch gate. 487 blocked
worker-turns have an adjacent next distinct stance with a legal later
job, not equivalent to cost-free resequencing. Among sampled tube-turns
with a legal centered pet on a ready tube, 646 lacked an exact opposite
pair and 69 had a free exact pair; no ready-but-reserved exact pair.
Only one modeled centered pet window lacked shoulders and already had
an exact pair. This is an occupancy/staffing diagnosis, not proof that
any future alternate routing or unfinished tube could have captured.
No implementation of same-stance reordering.

## One-step factual pairing cross-check

See `pair/`. Reproduced all 30 exact output tapes a second time. Look only
at ready real legal tubes in currently safe occupied states. A new pairing
requires two distinct people to reach its opposite stances in at most
one outside step, preserving all submitted walks/walls, never entering
the tube body or gate. A loan uses only a physically pet-blocked FIFO
dot, exactly one step away, with an open return edge.

The 42 free one-step compatible tube-turns (21 cases, 43 distinct
case-local pet×tube encounters) were all already assigned by v25 and
all after ordinary standby eligibility. There were zero additional
compatible pairing opportunities under this screen, including blocked-dot
loans. Seventy-three on-stance rows fail command compatibility because
they ask for ordinary waits rather than replace the already issued
same-turn closure; they are not missed moves. Do not infer that no
forecasted pre-alignment can help; the audit deliberately only studies
currently centered animals. No live guard overlay is justified by these
data.

## Partial-construction pair limit

See `partial/`. Counted exact games all30 a third time. At t35–100,
partial-mode cap1 picked a job on 13 sampled turns across five boards.
There was not even one unfiltered simultaneously forecast positive
second distinct pair that preserved the actual first pair. In
particular, zero free compatible second pairs requiring a real move.
No change to this cap. These three diagnostics close the low-cost
same-stance / one-step / second-pair opening family on this set,
not alternatives that reshape earlier division of construction.

The source generators, diagnostic scripts, summaries and raw logs are attached in `evidence.tar.gz`.
