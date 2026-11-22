# Can the existing late generator offer a short multi-animal pocket?

Read-only audit on the current narrow rescue standalone. Enter solely
after the existing handoff on cases with **at least 11 reachable animals
measured at t160**. Used dev30 contains seven such cases. Inspect candidates
already enumerated by the existing selector, respecting the original
radius/exclusion gates and the original min-cut prune. No new candidate
generation or changed ranking/actions. Look for a candidate with at least
two actual currently reachable pets inside, not a wide protection cut.

The staged filter first tests at least two reachable pets in the intended
footprint, then floods the actual board assuming the remaining planned
walls are closed (1–6 missing jobs). Require at least two of those same
inside pets to disconnect, keep all humans in one component, and retain
at least 200 and at least 65% of the prior human-reachable area. Require
a distinct eligible builder per wall job, with a real stance reachable
within five moves entirely inside the projected human exterior. This is
a deliberately narrow, conservative staffing screen; it does not test
multi-job crews, new geometry, route-persistent designs, waiting for
future arrivals, or longer plans. Even an admitted future path would
not prove simultaneous gate legality or predict animal stays.

Thirty exact selector calls across seven qualifying cases:
3,616 nominal two-pet candidates → 15 graph-feasible geometric candidates
across six calls/four cases → **zero short eligible exterior assignments**.
These are candidate/call counts, not unique geometry or captures. No
already-enumerated alternative met the activation gate (at least five
novel distinct turns across at least four cases). All seven complete
action hashes and scores matched the uninstrumented narrow comparator
and revalidated with official v3 playback and independent replay.
No live policy or new 100 cases; no promotion. This rules out reranking
the observed candidate pool under these guardrails, not wider multistage
co-designed captures.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-heavy-audit.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-heavy-feasibility.patch"
g++ -std=c++23 /tmp/ahc008-heavy-audit.cpp -o /tmp/ahc008-heavy-audit
python "$D/parallel_banks_heavy_feasibility.py" "$HARNESS" "$TESTER" /tmp/ahc008-heavy-audit "$DEV30" "$D/parallel-banks-empty-guard-dev30.csv" /tmp/heavy.csv
```
