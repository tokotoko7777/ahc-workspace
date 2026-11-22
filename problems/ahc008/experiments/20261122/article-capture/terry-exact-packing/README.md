# Exact singleton/pair room packing helper and tape-audit derivation

No change to frozen faithful source. For each room, deduplicate worker
masks down to the best positive integer score, then propagate maximum
weight over actor-usage subsets using exactly zero or one job in the room.
Transition visits only submasks disjoint from the job. Time
O(J + R_active*2^M + sum_over_unique_options 2^(M-job_crew)),
space O(J + R_active*2^M); M<=10 and pair/singleton masks. Scores and
old proposal ordering untouched on a tie or worse; the result carries
an empty override vector unless strictly better. Standalone fixture:
```
g++ -std=gnu++23 -O2 ahc008_terry_exact_packing/unit.cpp -o /tmp/packing-unit
/tmp/packing-unit
```
An odd roster with one pair and a residual singleton tests the invalid
old pruning premise, plus room/mask duplicates and 4000 random small
exact subset enumeration comparisons.

Generate the separate observation-only derivative and run offline:
```
python ahc008_terry_exact_packing/make_audit.py
g++ -std=gnu++23 ahc008_terry_exact_packing/audit.private.cpp -o ahc008_terry_exact_packing/audit
python ahc008_terry_exact_packing/run.py > ahc008_terry_exact_packing/report.json
```
Pins d14f6471... faithful faster source, and never modifies live actions
or RNG. Hunting telemetry is enabled solely for real t0–274. Offline
`--pairs=0` disables virtual MC (only from t275), preventing the
instrumentation's extra compute from affecting covered decisions.
Exact original input/action SHA and independent replay score verified
before using each printout; action bytes compared through t274.
No official tester and no claim that post-switch output follows original.

An alternate strictly better job list is emitted starting from the same
constructor reservations, then goes through actual deterministic idle and
sanitizer on a copy. Compare to the submitted historical command. The
Python screen verifies current legality and actual simultaneous geometry
for both choices, then separately counts new submitted singleton walls,
new actual pet isolation from that singleton's interior, their collateral
loss of any other observed exact catch, and retaining all old wall
commands. Counts are distinct turns/cases or singleton wall-events as
labeled, not score improvements or altered trajectories.

Practice100 result: verified all 100 exact action prefixes through t274.
Across 5,297 actionable singleton-candidate turns on 99 boards, the exact
objective strictly improves 43 turns on 25 boards. Resulting submitted
first commands differ on 38 turns (25 boards), but add **zero** currently
legal singleton walls and lose zero exact observed stock isolations. The
gate demanded both ten changed boards and five new no-displacement
singleton seals; the seal half fails. No live candidate or official run.
