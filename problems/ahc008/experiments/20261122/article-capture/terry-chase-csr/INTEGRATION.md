# Exact Chase CSR integration: retain fixed-work result only

Accepted f83fd834 versus candidate 9ba280d4; input, source, binary and
output hashes saved separately. Commands in GATE.md. All arms serial;
nothing rerun after observing results. No official submission.

Fixed: all 400 official runs (200 fixtures x two arms) match accepted full
action SHA and score at frozen per-case work. Whole-panel CPU reduction
25.64% distributed and 25.10% independent; positive-pair CPU reduction
37.21% (14) / 34.59% (17); fixed admission passes. Non-rollout variation
−0.06% / +1.49% CPU is unimportant and not claimed as a speedup.

Native 2670ms no cap: all 400 official, playback and independent checks
agree. Distributed 6,152,805,559 → 6,151,694,448, full/useful 100/100 both;
stock/candidate max wall 2.747/2.710s. Only changed case 0022, 47→83 pairs,
normal→Chase, 62,444,444→61,333,333. Independent 5,998,857,137 →
5,999,412,692, full/useful 97/97 both; max 2.753/2.708s. Only changed
case 0090, 27→55 pairs, Chase→normal, 27,555,556→28,111,111. More
virtual work caused clocked comparison to evaluate additional rollouts;
fixed parity excludes same-work action drift. These two live snapshots
produced different mode selections; no claim that a given selection
universally improves score.

Native is NO-GO under the prespecified distributed score floor and paired
nonregression test. Keep f83 default unchanged; archive CSR as a proven
fixed-work/kernel speed alternative only. No clock/cap/seed tuning.
