# Exact 2×2: choose the final gate during actual construction

Apply this comparison atop the previous square pilot. Use exactly the same
per-case shape location, support geometry, initially scheduled people and
setup/watch cutoffs as the fixed version. Retain the original scouting and
assignment data, which the harness compares byte-for-byte between pilots.
Only construction scheduling differs:

- The two initially assigned shoulder workers can redirect to any needed
  wall with an exterior stance, matched by legal immediate builds first
  and actual shortest outside walking steps second. They are no longer
  assigned an ordered wall subset. No ordinary solo preparatory assignment
  reserves more than two workers at a time.
- The third originally assigned guard can participate when at most three
  wall cells remain, but is not sent to the original fixed door. Keep at
  least one opening if an unoccupied pen would be sealed.
- If actual current positions admit simultaneous legal completion and a
  resident is inside, take that action. Otherwise use available shoulder
  jobs and reconsider observations next turn. Use the same setup deadline
  t36, 27 ready turns and t68 absolute stop as fixed square. The legal
  candidate still goes through original whole-team cohesion repair.

All 90 control/pilot outputs on the original capture-dev30 cohort were
official-v3 and independently replay verified. Max tester wall .537s.
This is the same seed set that informed the original shape and scheduling
hypothesis, so no promotion, holdout, or official distributed claim.

| Metric | Tactical control | Fixed square | Deferred square |
|---|---:|---:|---:|
| Pilot exact-4 pets | n/a | 10 | 13 |
| Pilot sealed pens | n/a | 9 | 12 |
| Final exact-4 pets, all pens | 4 | 10 | 14 |
| Final reachable pets (total over 30) | 165 | 145 | 128 |
| Final all-clear cases | 4 | 5 | 6 |
| Median first wall turn | n/a | 4 | 3 |
| Shells reaching single open gate | n/a | 30 | 29 |
| Median first single-gate turn | n/a | 17 | 15 |
| All shell builds, including final gates | n/a | 151 | 151 |
| Assigned crew walk commands | n/a | 945 | 963 |
| Assigned crew blocked/hold commands before single gate | n/a | 182 | 40 |
| Diverted ordinary commands | n/a | 1994 | 1146 |
| Diverted ordinary wall commands | n/a | 189 | 104 |
| Captures per 1000 diverted | n/a | 5.02 | 11.34 |
| Staffed occupied closure turns ignoring pet legality | n/a | 34 | 22 |
| Staffed occupied closure turns with pet legality | n/a | 12 | 12 |
| Of legal closure turns: 2+ open wall cells | n/a | 3 | 0 |
| Official development score | 603,294,259 | 485,985,124 | 592,642,545 |

Species of pilot captures, fixed→deferred: cows 3→4, pigs 2→4, rabbits
5→5, dogs 0→0, cats 0→0. Ten of the 12 actual deferred seals use a last
wall different from the original chosen gate. All 12 actual seals have
just one outstanding wall at execution. There is no evidence here that
multi-wall simultaneous completion caused the gain; the removed fixed
guard appointment and flexible shoulder assignment changed which single
opening remained. The same total physical wall investment and very similar
walking mean the efficiency comes mostly from returning otherwise idle
people to tactical, not saving wall material or routing distance. The
raw skill of late door alone is not isolated from flexible early matching.
Do not generalize to other families or cats/dogs until validated independently.

The closure opportunity metric uses every actual exterior worker, not
just the three assigned; if staff could cover all remaining walls, it
counts even if an original policy chose not to. Legal closure adds all
official occupancy/adjacent-pet tests. Replay verifies the exact four-cell
component for every captured pet after the final build.

Reproduction: compile the unmodified tactical control. Apply
quad-live-pilot.patch, then quad-watch-on-pilot.patch, then
quad-deferred-on-watch.patch in a scratch worktree before building the
GNU C++23 pilot. Pilot flags `2 27 0` keep fixed square, `2 27 1` defer.

    python quad_deferred_bench.py "$HARNESS" "$TESTER" ./tactical ./quad-defer "$CASES" results.csv
    python quad_deferred_gates.py ./quad-defer "$CASES" results.csv > gates.csv

The gate probe passes only initial visible setup without the seed or any
future pet replies, then reads the original fixed gate metadata. Case-level
official scores, raw replay derived telemetry and digests are archived.
