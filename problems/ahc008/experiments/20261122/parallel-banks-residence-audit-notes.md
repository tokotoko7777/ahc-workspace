# Passive residence at real optimistic pocket readiness (read-only)

On frozen narrow rescue, observe each newly selected ordinary fallback
target and up to three different existing high-score alternatives;
nothing about selection or future pet positions is fed back into play.
All 30 official scores/full action hashes and independent recorded
playback match the frozen controller. 146 selected episodes and 438
alternatives recorded, 448 total evaluable candidates; of selected
candidates 119 have passive animals in the eligible cohort (median ETA
8; only 42 observed all remaining jobs completed within 25 turns).
At nominal ETA 52 of the 119 have every not-yet-built wall globally legal.

Physical pen inferred by flooding the intended cells in the **current**
grid after the proposed remaining wall cells close, excluding giant
ambiguous shapes. Readiness is an optimistic lower bound: max of
nearest exterior stance actual passable-graph ETA + one build, and
jobs divided by available crew rounded up. It does not assume an exact
future crew assignment, waiting reservations or pet clearance. Propagate
each observed reachable cow/pig/rabbit from its actual cell through
the current fixed grid using all 1/2/3 uniform legal substeps. Measure
held endpoint inside target or legally away from the original or observed
still-open gate neighbors. Forecasts do not use future human walls, pet
steps or fallback decisions. Dogs and cats are counted separately and
not included in the movement forecast. Alternatives' measured outcome is
under the original actual controller, **not** a counterfactual capture.

* Own readiness: rank actual passive inside among alternatives, 169
  non-tied pairs in 24 cases: Manhattan 66.9%, exact DP 87.0%.
  Area alone 87.6%; current inside count 59.5% (same-evaluable
  common-ETA subset values differ slightly). Similar conclusions for
  actual-final-gate tagged safety.
* Force same horizon within each comparison group (the original chosen
  pocket's optimistic ETA) and recompute every alternative. Inside
  sort 71.9% Manhattan, 92.6% area, 97.5% DP, but DP beats area on
  only 7 distinct cases. Original-job-safe 68.3%, 92.5%, 94.8%, only
  5 cases better than area. Chosen-vs-alternative tagged-safe area
  slightly exceeds DP on the shared time label.
* Equal wall jobs/own readiness within 2 turns, common time:
  13 informative inside pairs in 10 cases, area 84.6% vs DP 92.3%
  (2 cases better); 16 tagged-safe pairs in 10 cases, area 84.4%
  vs DP 93.8% (3 cases better). Add equal current occupants and area
  within 20% and just 4 inside / 6 safe pairs remain.
* In already selected targets, ranking individual animals' actual
  inside is at 97.7% with legacy and DP; safe-tag metrics do not improve.

So broad raw gain mostly reflects geometry and forecast is not proven
useful across at least ten comparable cases. No live reranking or new
seeds; not an argument that pet dynamics never matter. The gzip ledgers
preserve intended/currently physical geometry, jobs, crew and original
ranks, ETA, species cohorts and model/observed per-pet quantities.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-residence-audit.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-residence-audit.patch"
g++ -std=c++23 /tmp/ahc008-residence-audit.cpp -o /tmp/ahc008-residence-audit
python "$D/parallel_banks_residence_audit.py" "$HARNESS" "$TESTER" /tmp/ahc008-residence-audit "$DEV30" "$D/parallel-banks-empty-guard-dev30.csv" /tmp/residence.jsonl /tmp/residence-summary.json
```
