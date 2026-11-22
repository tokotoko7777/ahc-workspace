# Selective deferred square: one pre-registered visible forecast gate

Compare unchanged tactical, the full deferred square, and its selective
version. Use only the old 30 development inputs and exact official v3
action/replay verification. Dev results are exploratory and were not
tuned on either 100-case validation cohort. No new unseen run.

Keep the same normally selected square and assigned people and compute
the actual shortest outside-only routes to every original shoulder/guard
appointment. Builder ETA is the maximum over the guard path and each
assigned shoulder-worker route plus its build turns. This is a geometry
estimate without temporary pet blockage, and deferred matching can choose
other jobs once active. If the ETA plus two fixed setup turns exceeds the
safety deadline, decline to construct. Otherwise run exact uniform passive
cow/pig/rabbit substep marginals on the initial passable grid. For each
legal exterior gate orientation having both checkerboard-safe colors, and
turn ETA+2 through ETA+5, compute the chance at least one passive pet is
in a closure-safe pen cell. Use the maximum of these individual snapshots,
not a joint probability of capturing within the window. Gate approach,
temporary build delay, other pets' blocking and the effect of future
shoulder walls on those distributions are not predicted by this gate.
Dogs and cats are not forecast. Do not rank-relative select a forced shell:
the previously selected geometry either clears the gate or is omitted.
When activated, execute exactly the already frozen deferred-square policy.

The pre-registered criterion was ≥20% forecast at one candidate gate/time.
Dev acceptance needed ≥80% of full direct captures, ≥25% improvement
in captures per diverted command, and no aggregate score or residual
regression against full deferred. No threshold sweeps after seeing results.

| Measure (dev30) | Tactical | Full deferred | Selective |
|---|---:|---:|---:|
| Shell policy active | n/a | 30 | 1 |
| Direct exact-4 pets | n/a | 13 | 1 |
| Final exact-4 pets | 4 | 14 | 5 |
| Final reachable pets | 165 | 128 | 161 |
| Displaced ordinary commands | n/a | 1146 | 21 |
| Replaced wall commands | n/a | 104 | 2 |
| Direct exact-4 pets per 1000 displaced | n/a | 11.34 | 47.62 |
| Official score | 603,294,259 | 592,642,545 | 606,579,849 |
| Max official tester seconds | .510 | .520 | .603 |

All 90 runs were legal. The sole qualifying case was 0011: ETA7,
forecast .327382361028. Its pilot closed turn13 and captured one pig;
official score 14,825,521→18,111,111 and residuals 4→0. All selective
action bytes and score there are identical to full deferred. Across
29 omitted cases, every selective action byte and score is identical
to original tactical. On those omitted cases, full deferred captures
12 direct compact pets and uses 1,125 displaced commands; its score
574,531,434 and 128 residuals become 588,468,738 and 161 on tactical.
Median forecast .09985; next highest .16223. Efficiency rises on a
single case but retention is 1/13 and residual count worsens +33 vs full.
The apparent aggregate score win over tactical is only that single
case. Reject this trigger, keep both established solvers unchanged,
skip unused cases. This rules out this early four-turn open-board gate
as a robust operating criterion, not all readiness-informed policies.

Reproduce by building control separately and then applying
quad-live-pilot.patch, quad-watch-on-pilot.patch,
quad-deferred-on-watch.patch, quad-selective-on-defer.patch in order
to a scratch copy of tactical. Build the pilot in GNU C++23. The final
flag selects the forecast gate; prior shape/watch/deferral are 2/27/1.

    python quad_selective_bench.py "$HARNESS" "$TESTER" ./tactical ./quad-select "$CASES" results.csv

The archived per-case CSV has initial forecast, selection, exact score,
legality via success and replay, species counts, and transcript hashes.
