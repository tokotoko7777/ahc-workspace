# Live overlap of exact-four pen with ordinary early dog fence (diagnostic only)

Same already used tactical control dev30, all source input/action hashes and
official scores unchanged. Replay every state under official v3. No solver
actions changed and no holdout tests. Enumerate all 19 oriented tetrominoes
translated against any board edge. Candidate needs at least two reused
ordinary dog-fence cells, at most two new boundary cells, an exterior
stance for a final gate with both legal checkerboard colors, and the larger
dog gate stays open and connected from its corner to the main board upon
completion. Reused cells may be diagonal neighbors; no assumed straight
side. Track motifs only once their shared walls are actually built.

| Stage | Motif snapshots or cases |
|---|---:|
| Cases with actual original dog-fence plan | 26/30 |
| Eligible static motifs | 52 across those 26 |
| Eligible motifs outside the large fence | 0 |
| t0–69 shared walls actually built and large gate still open | 2502 |
| Same with an observed pet inside the small pen | 125 across 18 cases, 25 distinct case-pet IDs |
| Also no humans inside or on prospective pen walls, dog-to-main connectivity preserved | 122 |
| Also official pet adjacency permits all one/two new walls now | 46 across 14 cases |
| Also enough distinct exterior builders already on their stances | 1 |
| Also preserve all ordinary human jobs without extra relocation | 0 |
| Also no other pet currently left inside the large corner fence | 0 |

Exterior means outside the compact pen, not necessarily outside the large
dog fence. Match immediate wall jobs exactly across actual people. A
worker doing the same pending fence build or truly idle can be assigned
to a no-displacement match; unchanged moves into proposed walls or pen
invalidate that claim. Do not assume an existing builder stands behind a
wall or can walk and build in one action. An original large gate cell ends
up blocked in 23 of the 26 dog cases (whether by dog plan or later tactics);
a small pen contained within that later closure may reclaim no score area.

Only staffed witness dev0012 t16: pig index15 at a 4-cell T-like edge pen
`[(0,7),(0,8),(0,9),(1,8)]`, needs `(0,6)` and `(1,7)`. Exact observed
pet clearance and builder stances permit simultaneous closure, but
ordinary jobs would be displaced, and three other current pets are in
the large dog region. This cannot replace the broad gate. No live overlay
tested or recommended; static wall sharing solves material cost, not
current worker coincidence or whether the expanded clean area survives.

Reproduce using the unchanged instrumented binary from quad-causal-notes
and the exact matching dev30:

    python dog_fence_quad_audit.py "$HARNESS" "$TESTER" ./instrumented "$CASES" quad-defer-dev30.csv witness.json > audit.csv

## Borrow a stance already on the approach to the anchor

After all actual original dog walls exist and while phase1 is active,
scour ordinary paths before turn155 for workers already on either T-pen
wall's exterior stance. A normal movement could be postponed one action
to build, but no extra walking and no original build displaced. This
is not free: a movement delay would change later observations. Search
for a complementary second stance with target inside and both wall
actions locally legal on the observed trajectory, within eight turns.
Record if the original paths would cross the added first wall, and
whether any other animal remains in the broad corner. These are
hindsight route windows, not counterfactual simulation of pet motion.

Across 679 actual post-prep phase1 turns in 24 cases, there are 43
geometric stance visits, 40 individually legal single-wall jobs not
displacing a wall command (zero on genuinely idle actions), and six
with a resident at a potentially finishing stance. No complementary
pairs fit the eight-turn calendar even before rejecting path crossings
or checking whether the broad parent gate should still close. Skip
rollout. Same command above, add `--routes` and unique outputs.
