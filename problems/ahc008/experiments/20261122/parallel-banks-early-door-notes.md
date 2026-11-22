# Optional one-then-one gate sequence on actually occupied banks

The primary policy remains the user's simultaneous two-mouth capture.
First audit the staged 3×12 dev30 traces for an alternative: brace ready,
both mouths genuinely open, an observed cow/pig/rabbit in a middle two
cells, no human in the tube, current legal build from a staffed near
mouth, and a different human who can reach the opposite exterior stance
in ≤2 steps on current passable board after excluding the mouth and body.
Skip any tube already eligible for a normal simultaneous closure. There
are 88 ETA≤2 windows, 49 distinct case-pets in 23 cases, 17 otherwise
unbanked across15. ETA≤3 adds only one such unbanked animal. Requiring
t≥101 still gives 63 windows, 33 distinct, and 12 otherwise unbanked
across12, passing the pre-registered 12 animals /10 cases threshold.
Feasibility counts are conditional on earlier observed traces, not
forecasts or evidence for actual simultaneous outcomes.

One matched live original dev30 A/B: only post-t101 and at most one
pending early-door reservation at a time. Keep original phase/fallback
and immediate simultaneous closures in their original priority. Near
guard builds now with full legal occupancy checks, then reserve opposite
guard for at most ETA+2 turns and finish only with an actual resident
and legal far door. Never close on empty or with an enclosed human.
Abandon a failed reservation without rebuilding or reopening the wall.
All 60 outputs legal and exact official replay; reference reconfirms
all 30 old staged action digests, same geometry and both exact prefixes
through t100. All actual humans/service aisles connected bank phase
through t225 in both arms. Replays double-check exact component area
4 for every credited closure, regardless of whether atomic or staged.

| Metric (dev30) | Ordinary simultaneous baseline | Supplement early door |
|---|---:|---:|
| Actual opened halves | 0 | 34 |
| Exact-four staged finishes | 0 | 22 |
| Staged pets | 0 | 23 |
| Failed permanent first walls | 0 | 12 |
| Initial occupant incidences leaving half-open body | 0 | 16 |
| Atomic seals | 210 | 185 |
| Atomic pets | 219 | 192 |
| All direct exact-four bank pets | 219 | 215 |
| Direct species [cow,pig,rabbit,dog,cat] | 23/37/38/77/44 | 25/39/33/74/44 |
| Residual at t225 → final | 216→174 | 220→157 |
| Final safe human union total (distinct cells per case summed) | 976 | 1484 |
| Final all-clear cases | 2 | 5 |
| Official score sum | 232,815,017 | 300,260,862 |
| Maximum tester seconds | .621 | .564 |

Direct compact paired wins/ties/losses 7/15/8. Score 11/12/7.
Nine whole cases preserve all action bytes. This creates real short
sequential captures, but displaces enough atomic outcomes to lose four
direct captures, in part by changing future pet dynamics and lane
travel. Aggregate score/clear improves on this development cohort
mostly **after** the unchanged generic fallback (63 pets removed vs
42), not at the bank release. It is not robust enough for automatic
promotion or to replace the conceptual simultaneous mechanism. Keep
separate as exploratory; no new heldout cases.

Apply parallel-banks-pilot.patch, parallel-banks-staged.patch, and
parallel-banks-early-door.patch to scratch tactical. Build GNU C++23.
Pilot flags 3 (prior) vs4 (early door) for this separate executable,
not the wide geometry source, whose CLI4 means a different variant.

    python parallel_bank_early_door_audit.py "$HARNESS" "$TESTER" ./banks-staged "$CASES" parallel-banks-staged-dev30.csv > eligible.csv
    python parallel_banks_early_door_bench.py "$HARNESS" "$TESTER" ./tactical ./banks-early "$CASES" compare.csv
