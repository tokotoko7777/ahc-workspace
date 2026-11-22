# Braces as a late transverse fallback: geometry/worker lower-bound audit

Only examine exact already-used staged3 dev30 traces at t205 and t225,
no new output policy. Verify official action hashes and scores against
frozen dev30, then exact replay. For each of the 13 transverse brace rows
and each side of a hypothetical full continuous row, compute exact
passable connected retained area and number of observed pets. Count all
missing wall cells and separately cells forbidden at that snapshot by
pet occupancy/adjacency and human occupancy. Keep service-lane-reachable
parts, not pets-only components. The table contains 1,560 side snapshots.

For each side at least 120 cells, at most one pet, compute an optimistic
crew schedule lower bound: actual shortest passable routes to any
retained-side stance for that wall, exact distinct worker-time slot
matching (one build per worker per turn, after initial arrival), and
a minimum bound for everyone to reach the chosen side. This deliberately
drops inter-job walking, path changes caused by walls, arrival
coordination, future pet movement and future clearance. Requiring no
currently pet-blocked cell is a separate initial screen, not a proof that
the jobs stay legal throughout. No worker-time result is stored for an
unqualified area/occupancy; sentinel999 also means >30 or met no stance.

| Number of cases with a feasible-*geometry* side | t205 | t225 |
|---|---:|---:|
| Clean ≥120 | 12 | 15 |
| Clean ≥200 | 0 | 2 |
| Clean ≥300 | 0 | 1 |
| ≥300 with at most one pet | 6 | 7 |
| Same and worker lower bound ≤25 | 5 | 6 |
| Same plus no currently pet-blocked required cells | 4 | 3 |
| Clean ≥300, same worker and blocker screens | 0 | 1 |

Even after brace reuse, full transverse separation needs 18–30
additional cells (median18 for score-relevant sides). At t205 maximum
clean possible is 198. At t225 the only case yielding clean ≥300 is
0020; best ideal 582 cells is currently blocked and even optimistic
lower bound26. Its strongest clean row meeting the screens holds 534
cells and lower bound24, but ordinary fallback already holds an actual
670-cell safe union in this case. The at-most-one-pet cases generally
do not resolve the score target (each pet halves that area's score).
No live rollout or tuning on these cases.

Reproduce from repo root using recorded staged dev30 and the GNU C++23
pilot from parallel-banks-staged-notes.md:

    python parallel_bank_brace_audit.py "$HARNESS" "$TESTER" ./banks-staged "$CASES" parallel-banks-staged-dev30.csv > brace.csv
