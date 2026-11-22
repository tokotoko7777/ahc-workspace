# Bounded original-map scout: proxy preflight not strong enough

With the accepted incoming policy on new uniform generated100,
only consider original top1 == map1; compare map1 against
map2 with the identical initial board/actors and two paired
fixed ex ante simulation seeds. Four complete Normal-only
trajectories must stay within a shared 900ms consumer budget,
finish full in every arm, and retain more floor in both pairs
for the alternate to be preferred. No real future inputs or
actual solver commands are sent. No late policy evaluation;
these are simulated proxy results only.

22 cases qualify, 9 finish both pairs (40.9%, versus ≥80%),
13 only one pair. Only 3 would choose map2, versus ≥6, and
their summed *minimum of two paired* mean-floor advantage
is 153 tiles (1.53 per board across all100, versus ≥400 total
required). Ordinary timing budget did not permit four complete
trajectories reliably; median eligible observer body 900.18ms,
max 902.83ms including up to the already-started operation.
All cases followed their fixed seed scripts and input hashes.
No real official tester score or causal capture change measured.

Reject this design without adjusting budget, candidate maps,
simulated arm count, or using partial results. Current accepted
solver, captures and distributed score remain unchanged.
