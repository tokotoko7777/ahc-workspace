# AHC008 v25: factual lifecycle of the new early seams

On the original M5-dev30, the audit binary reproduces all 30 v25 input
and action hashes and official scores. Independent tape playback agrees.
Across 23 cases, the new phase selected 99 distinct line episodes;
only six physically closed all requested cells. All six real closures
were on turns without unrelated new walls; these transitions isolated
15 actual pets in total. Of 93 non-completions, 62 placed none of their
owned walls, and 70 lasted one or two turns. In this set, 298 turns were
owned by an early chosen line, with no post-processing override on
logged routes. Most non-completions occur before many walls are built,
so counting static splits as successful would greatly overstate effect.

The original episode trace is stored as compressed JSON. The raw audit
uses the unchanged source timing-sensitive code plus observation logs;
no policy, score or output changes are intended. Outputs happened
to be identical for all cases under the recorded conditions.
The separate extension of the admission area floor after turn 205 is
not adopted; see [persistent/README.md](persistent/README.md).

Regenerate the diagnostic with `python audit/make.py PATH_TO_V25 solve.cpp`
from [pinned v25](../v25-early-seam/README.md), then compile
`g++ -std=gnu++23 solve.cpp -o solve`. Set `AHC008_EARLY_SEAM_AUDIT` to
a separate log file per run. `audit/replay_audit.py` reads an official
input, actual actions, official animal replies and the audit log.
The original `audit/run_dev.py` retains its original workspace paths.

The six real completions retained 645–698 mean human cells at closure.
Their admission estimates allocated a mean of 65.2 cells of the whole
root away from the human side, vs 20.1 among the 62 episodes that placed
no requested walls. This is an observational small-sample contrast,
not a cutoff to tune on.

As a separate ablation, rerank the top four eligible early seam shapes
by estimating whether at least two target animals remain outside the
preferred human side at the estimated wall completion time. Use 24
shared Monte Carlo samples under unchanged walls and people. On the
same dev30, score 536,821,892 → 484,372,545, full 11 → 9, useful full
4 → 3, residual 79 → 81; nine changed histories. All 60 official
and independently replayed. No promotion or held run. This statically
forecasts motion, not the altered geometry during construction.
The generator and data are under `early-forecast/`.

Last ablation: only after at least one of an early seam's planned wall
cells is observed closed, let the remaining plan continue while one
rather than two pets would still be separated; all area/benefit and
legality checks stay. This did not salvage the short plan family:
536,821,892 → 495,841,315, whole 11 → 9, useful whole 4 → 3,
last residual 79 → 82, 12 changed histories, maximum local tester 0.602 s.
All 60 official and independently replayed; no held run or adoption.
See `started/`. Keep v25. Prefer a differently prepared capture region
over another cancellation threshold in the same early seam family.
