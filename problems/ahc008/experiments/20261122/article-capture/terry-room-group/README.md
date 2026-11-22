# One-turn normal room grouping: impossible in fixed geometry

Reproduce with `python ahc008_terry_room_group/screen.py` from the
workspace root. It verifies full input and action hashes and independent
score replay for every practice100 tape. No solver or official tester.

Use genuine occupied normal-mode components newly isolated from all
humans, where every actual new boundary wall is one of one or two configured
mouths with the actual worker on the outside. Replace just those lowercase
actions with waits, retaining all non-guard actions and all other walls.
The original workers can keep the exact same stances for a one-turn delay.
Restrict outsiders to previously human-accessible animals outside this
component that were actually separately isolated in a later stock turn.
There are 1,427 genuine occupied room episodes, of which 1,336 have at
least one such outsider; 10,328 episode×outsider pairs.

For an actual wall to be legal, each outsider at t is neither on nor
orthogonally adjacent to any old gate, so reaching the first gate takes
at least two pet substeps. As every other wall is kept, any entering path
must cross one of these gates. The endpoint in the intended compartment
must likewise be neither on nor adjacent to a new gate build, so after
gate entry it takes another two pet substeps. No species can make four in
a turn (cow/pig/rabbit 1/2/3, dog/cat 2). This is exact for passive and
directed species already at path-support level, without independence or
waiting-incumbent assumptions. The script separately confirms zero
outsiders within three graph steps of the gate-safe inside on all tapes.
No positive-probability edge exists; distinct later compartments spared,
saved-area upper bound, and joint one-turn capture probability are zero,
so no matching is needed. Different gates, omitted non-guard work or a
longer delay are outside this test, not proposed.

## First physically possible lag: exactly two turns

Run `python ahc008_terry_room_group/two_turn.py`. Same exact tapes/mouth
guards, but freeze all non-mouth t walls while those mouths remain open
through two pet moves. The first geometry bound allows waiting within
twice the species speed (including impossible parity endpoints), ignores
other pet blockers, and requires all incumbents can reach a safe room
endpoint. Credit a later physical **terminal** compartment's floor area
only when each of its final occupants is a distinct eligible later entrant
at that same host event. Each disjoint final component credited at most
once per board; choices may otherwise conflict with one another. The
bound is on these frozen-board witness floors, not a bound on every
possible counterfactual. It also excludes any extra canceled-wall area.

Optimistic geometry: 266 eligible room events on 87 boards, 320
event×entrant pairs, 238 distinct later terminal compartments, 1014 floor
cells in total. No cow entrant qualifies. Exact passive endpoint
probabilities on the fixed walls (one/two/three unbiased neighbor
substeps per turn), multiply required passive incumbents and newcomers,
ignore all unrelated pet blockage. Take max probability over hindsight
eligible hosts for each final-room credit and sum best expected area.
For fully passive participants, 93 terminal compartments have positive
probability, yet the generously selected sum is only **1.596 expected
floor cells across all100**. For all opportunities even replacing every
directed participant factor by one, the same sum is only **361.071**
expected floor cells (still ignores external blockers and actual policy
interactions). No live change is recommended: the distance-only
potential is real but largely unavailable under actual passive movement,
and even a very permissive directed-species bound is far below the
~1070-cell project target.
