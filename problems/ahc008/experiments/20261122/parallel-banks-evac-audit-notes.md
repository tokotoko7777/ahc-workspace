# Current narrow rescue: which wide cuts underestimate crew evacuation?

The original `latePlan` optimistic cut ETA estimates closest builders and
wall-job throughput. True execution later asks all mobile humans in the
selected current component to enter the retained post-cut part, while
non-mobile groups/captive humans are not evacuation targets. This audit
checks the exact passable-graph distance to the **actual kept component**
used by the executor at initial line selection. It separately validates
routing before and after the actual same-turn walls by exact recorded
replay, not the initially planned full cut. It is not a simulation of
what a different route or ranking would have produced.

All 30 existing dev narrow action hashes and full scores reproduced
byte-for-byte with official v3 and independent replay. 2,267 selected
line ticks; 486 new commitments across 29 cases. 227 commitments
require evacuation. The preregistered substantial condition was
shortest crew evacuation ETA ≥ original ETA + 5, or past the game
deadline. It occurred in 25 commitments across 16 cases:
9 finished and 16 did not finish (the latter across 10 cases). One
commitment was past deadline by evacuation distance alone.
Six commitments had zero currently available eligible humans on the
kept side; 69 lacked at least one reachable kept-side stance from a
human currently already there. The first number is purely about
starting positions, not a claim that those builders cannot arrive.
All mobile/kept/other groups were checked from the actual observed grid;
none of these bank rescue trajectories used the separate tactical decoy.

Actual recorded same-turn walls lengthened exit distance on 75 selected
ticks across 18 cases; none removed every exit. Prior raw line proposals
would remove exits on 71 ticks across 8 cases, but this did **not** survive
later safety repair to actual walls. Do not treat the 16 noncompletions as
proven caused by a bad ETA—pets, score reevaluation and lane blockage can
also cause a changed line. The recurrence screen supported testing one
fixed ranking-only ETA bound without changing geometry or commitments.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-evac-audit.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-evac-audit.patch"
g++ -std=c++23 /tmp/ahc008-evac-audit.cpp -o /tmp/ahc008-evac-audit
python "$D/parallel_banks_evac_audit.py" "$HARNESS" "$TESTER" /tmp/ahc008-evac-audit "$DEV30" "$D/parallel-banks-empty-guard-dev30.csv" /tmp/evac.csv /tmp/evac.json
```
