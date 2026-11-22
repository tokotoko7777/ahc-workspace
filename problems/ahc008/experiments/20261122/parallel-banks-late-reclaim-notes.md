# Existing paired bank gates after rescue handoff (read-only dev30)

Question: can ordinary fallback opportunistically close a prepared exact
1×4 trap when its two workers happen to be on the required outside stances?
No relocation, altered geometry, planning, source change, or A/B. This is
an audit on exact frozen rescue trajectories, not a counterfactual rollout.
All 30 original scores and action hashes match the frozen rescue dev CSV;
official v3 playback and independent exact score reconstruction also match.

Scan zero-based turn t from its actual 160/225 handoff to 299 inclusive.
Require open body, all shared braces built, two open gates legal against
all current animals and humans, at least one presently reachable animal in
the body and no human in the body. Then condition on two distinct existing
stance occupants, retaining every ordinary wall, and all humans connected
in their **projected positions after the candidate turn**. At that last
filter, verify the proposed pen flood is exactly its four cells and
disjoint from humans. A stricter test also retains all normal moves.
Repeated tube-turns are separate from distinct case/animal identities.

| Filter | Tube-turns | Cases | Distinct otherwise-final-unresolved animals | Cases with a miss |
|---|---:|---:|---:|---:|
| Geometrically/legal, ignoring staffing | 688 | 25 | 33 | 16 |
| Both workers already ready | 4 | 4 | 1 | 1 |
| And retain all normal walls | 4 | 4 | 1 | 1 |
| And preserve full post-action human connectivity | 4 | 4 | 1 | 1 |
| And no normal move lost | 1 | 1 | 0 | 0 |

Of the four staffed tube-turns, one (0022 t160) was already closed
atomically by default and contained two animals. Four distinct individual
animals from staffed windows end up already caught by ordinary behavior.
The only otherwise-final-unresolved one is dog index 5 in case 0019 at
t160; at least one existing move would be displaced. Future ordinary
capture/failure labels are for retrospective classification only, never an
online signal. Staffing, not geometry, is the observed bottleneck.
The predeclared A/B gate of >=5 novel unresolved animals across >=4 cases
failed. No live experiment, promotion, fresh100, or practice100.

```sh
D=problems/ahc008/experiments/20261122
g++ -std=c++23 "$D/parallel-banks-rescue-research.cpp" -o /tmp/ahc008-rescue
python "$D/parallel_banks_late_reclaim_audit.py" "$HARNESS" "$TESTER" /tmp/ahc008-rescue "$DEV30" "$D/parallel-banks-rescue-dev30.csv" /tmp/late-reclaim.csv /tmp/late-reclaim.json
```
