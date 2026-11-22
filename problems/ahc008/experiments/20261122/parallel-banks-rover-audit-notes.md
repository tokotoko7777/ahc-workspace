# Can an existing spare worker opportunistically finish a paired gate?

Read-only replay instrumentation, no live rover/patrol. Test the 22
M7/9/10 cases of the common old visible M>=7 dev30 (the eight M8 have
no never-zone-assigned worker). Verify exact t101 zone roster and the
entire baseline score/action hashes against the frozen narrow-rescue
default before counting. The roster has one spare in M7/M9, two in
M10, fixed through bank activity. All 22 probes are legal exact
official v3 action playback and independent score matches.

Consider post-t101 (through each t160/225 handoff) a truly prepared,
occupied and immediately two-gate-legal exact four, no human inside.
Require exactly one normal assigned guard on its actual stance and
no non-reserve worker on the other stance. Trace unused reserves
through up to two actual path steps, allowed only if the reserve
has no scheduled normal command for every approach or close turn.
Retain chronological walls and legality at both proposed start and
end, same observed legal animal ID throughout, same assigned human
present throughout. At closure keep all ordinary walls, and check
simultaneous ordinary other moves plus substituted pair keep all
humans connected outside that exact four. Report also a stricter
filter requiring the normal partner not to give up any scheduled move.
For an ignored-duty lower bound on difficulty, compute exact one/two
step reach on the current real grid separately. Future observations
are hindsight ceilings, not a simulation of changed target behavior.

There were **82** one-normal-position legal windows, involving seven
otherwise-final-unresolved distinct case/animal IDs *before considering
reserve positions*. Only one such window of any kind had a rover
within two steps even ignoring future occupancy/commitments (0031,
whose animal was normally caught). The rover there was initially
idle. After full observed target/partner and route checks, zero
candidates fit any of now/one/two steps, zero otherwise-final misses
and zero cases. The recurrence gate (12 missed case/pet IDs across
at least 8 cases with supporting near-term readiness) was not reached.
Moving a rover proactively to a different local assignment is a
structurally different and untested policy, not justified by this audit.
No live A/B, held-out/practice test, main/submission, or promotion.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-rover-audit.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-rover-audit.patch"
g++ -std=c++23 /tmp/ahc008-rover-audit.cpp -o /tmp/ahc008-rover-audit
python "$D/parallel_banks_rover_audit.py" "$HARNESS" "$TESTER" /tmp/ahc008-rover-audit "$WIDE_DEV/eligible" "$D/parallel-banks-capped-hybrid-eligible.csv" /tmp/rover.csv /tmp/rover.json
```
