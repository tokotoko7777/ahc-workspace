# Single partial macro-room tree: rejected development pilot

On the frozen cap4 control, substitute early closure of one cycle edge
of the fixed NW 2×2 macro square, leaving corner room 0 with its adjacent
south portal. Only propose if BFS on the actual passable grid after
prospective closure of both apertures isolates the occupied leaf inside
room 0. Reject all currently present or planned humans in the leaf.
Between t40 and t122, only while no refuge is active, alter at most
six single-worker commands and use at most three-step candidate routes.
Run through normal action repair and final exact protection.

| Metric | Frozen cap4 | Partial tree |
|---|---:|---:|
| Official development 30 score | 297,777,775 | 301,111,188 |
| All-clear cases | 30 | 29 |
| Pet-free humans | 219/219 | 214/219 |
| Max tester wall time, seconds | 0.220 | 0.242 |
| Mean safe union, shared 29 all-clear cases | 90.72 | 93.45 |
| Mean pet-only area, shared 29 all-clear cases | 657.10 | 654.38 |
| Mean pet-only area, all 30 | 658.53 | 632.57 |
| Mean mixed area, all 30 | 0 | 25 |

All 60 outputs official and replay verified. The pilot paid one reliability
loss in case 0014 (all 20 pets still reachable) for no attributable new
captures; the lower overall pet-only area is mostly conversion to mixed
area, not compaction. Exactly three extra cycle cuts occurred (t55/63/79).
Together these three traces exhibited 144 subsequent turns with an occupied
human-free leaf legally separable by the gate; none had a person already
on an outside stance. Both variants retain a separate baseline portal close
at t122 that isolates the same count (3). This design needs a different
staffing geometry, not just the same corner mirrored. No holdout or promotion.

Reproduce with partial-tree.patch applied to a copy of the frozen solver
and compile its pilot with -DAHC008_PARTIAL_TREE. Run the usual official
benchmark_cases.py with --capture-topology and --count 30, and use the
independent partial_tree_gate_trace.py with benchmark wrappers, official
tester, two binaries and dev30/in. Cut attribution checks reachability for
pre-move pets from actual human destinations with all same-turn walls
except/tested gate, rather than treating a mere wall build as isolation.

## Fixed service spine with six large lobes: feasibility only

Sketch a one-cell horizontal service spine along row 14 (zero-based).
Its two retaining wall strips use 54 cells after leaving six independent
access portals. Two perpendicular ribs per side (columns 10 and 20) use
42 cells if three tip cells per rib are left open, for 96 preparation
walls total. Spending all 12 tip cells later totals 108. Each adjacent
sibling has independent access through its door; initial ribs permit
short 1–3-cell local closures but do not themselves enforce separation
across the gaps.

The service oracle projects this fixed geometry onto existing verified
capture4 times 40–210 before first seal. It grants full 756 lobe area
before occupancy, does not charge initial staff/legality/disconnection
or added wall construction, and does not alter pet trajectories. Best
sterile lobe/door sum averages 216.5 (7/30 ≥300; maximum396). Requiring
that the service spine also be empty gives 171.6 average (4/30 ≥300;
maximum373). Adding all 30 spine cells still leaves every projected
case below 450. This is underpowered as a fixed launch layout on
observed trajectories, not a proof about other shapes or altered
dynamics. No production implementation. Reproduce with the archived
sterile_spine_audit.py --service-oracle and same solver/data.
