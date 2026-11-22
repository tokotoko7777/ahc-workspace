# Moving cow sweep: observed capture, insufficient efficiency

This is a different topology from stationary notches. It identifies one actually observed slow cow close to an existing barrier, and searches a very small vertex cut to physically dispose of its current tile and potential one-step neighbors. On each turn the front can move to the latest actual position, while walls already constructed remain useful as anchors. Use no future seed, action, or position.

The precommitted limit is 3 controllable human commands in the current turn and 26 actually changed worker-turns total, at most three target attempts, a 10-turn deadline per target, and absolute deadline turn 116. Stop as soon as the macro safe region is selected. Baseline wall placements cannot be changed; if connectivity repair would cancel one of them, revert the whole overlay. Extra walls are local, not part of all 16 seams. Compile capture_first_submission.cpp with the patch applied to a scratch copy (GNU C++23). Add -DCAP_SWEEP_DIAG for counters.

Same 30 official-generator development cases, official tester + independent recorded-action replay:

- Capture4 30/30 all-clear, 297,777,775; swept 29/30, 271,889,047; both all legal, sweep maximum tester wall 0.258816 s.
- Aggregate instrumentation: 57 cows targeted; 63 actual added wall actions; 53 targets escaped or were abandoned; four tracked cows isolated. Exact increment also four additional pets moved outside the human component versus the unchanged legal base command in the same state on the same turn, before animal stochastic movement. 444 changed worker actions, of which 414 replaced pre-existing human movement commands. No need to attribute final closure to future hypothetical moves.
- Same-clear 29 paired cases: human-safe union 91.17→84.38, total distinct pet-only area 656.66→662.31, largest pet-only component 638.93→658.31; every matched case has one connected human group.

Only 4/57 pursuits worked at about 111 changed worker-actions per marginal isolation. The dynamically relocated front showed actual isolation, but delay, diversion and newly blocking walls outweighed it. Do not run holdout or promote. Full source patch, counters, score and exact topology evidence preserved.

Further progress needs genuinely short joint actions: a real current two- or three-worker closure opportunity around already present natural barriers, with guard stances reachable simultaneously at most a single turn later. An observed-entry hazard policy may use a designated watcher only if measured near-term target occupancy justifies a small worker-time budget. Do not add another open-ended pursuit, static waiting grid or coefficient sweep. Keep the reliable capture-first evacuation and the separate tactical score leaders unchanged.
