# Equal-flow late cut feasibility audit, independent development only
[Protocol and eligibility](PRESET.md). The analytic kernel uses the original
shape builder with the same max-flow library and computes the second
residual extreme without a second flow. The audit instrumenter does not
modify either the live action or the traced pet motion. True stock
chase-only mode is verified by end telemetry. No original Rust, private
network coefficients or source are included here.

From workspace root, in an exclusive slot:
```
python ahc008_terry_chase_cut_ties/prepare.py ahc008_focus_assets/tools/target/release/gen
g++ -std=c++23 -Wall -Wextra -O3 ahc008_terry_chase_cut_ties/kernel.cpp -o ahc008_terry_chase_cut_ties/kernel
timeout 200s python ahc008_terry_chase_cut_ties/audit.py \
  ahc008_focus_assets/tools/target/release/tester ahc008_terry_passive_exact_short/live \
  ahc008_terry_chase_cut_ties/kernel
```
The frozen baseline executable, input manifest and seed list are pinned.
Official baseline materialization is one run per new board, plus official
playback and independent Python replay for exact legality and score. No
alternative trajectory is scored. A timeout/failure is incomplete, not a
negative result; always retain or explicitly report the locked denominator
60. Full report contains action/tape hashes. The kernel itself has a
separate 30-second bound. Do not reuse this dev60 to widen the test.

## Completed result: did not pass the structural gate

All 60 baseline-only runs were legal and matched official playback and
independent scoring. Seven selected real Chase, yielding 175 submitted
turn snapshots from t275 through t299. Reasons were 105 no remaining
majority target, 11 identical physical wall masks, 24 no newly isolated
animal with both cut masks, one non-dominant retained floor, 25 infeasible
staffing/ETA, and nine strict witnesses. The nine unique witnesses cover
four boards; their best per-board mean-worker-area increments are
5, 10, 6 and 7, totaling **28** rather than the required **50**. No
witness was an immediate build (minimum approach four moves). Nothing
here demonstrates pet retention after those moves or a score gain.
No live variant, official practice access, or scope expansion.
