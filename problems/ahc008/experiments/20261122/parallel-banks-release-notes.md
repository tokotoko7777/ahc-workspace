# Hunt on the already-built staged 3×12 bank until t275 vs t225

Single phase contrast on the original dev30. Use the earlier staged 3×12
policy, not the wider four-bank variant. Freeze geometry, staffing,
placement, forecasting and construction stop. Only change release from
bank hunting t225→t275, leaving 25 vs 75 turns for the same existing
tactical controller. Suppress the tactical-gate postprocessor during the
extended bank phase (it was already inactive before t225 original),
and run it normally during fallback. Exact terminal action remains at t299.

All 60 outputs legal and verified through recorded-action official replay.
All 30 reference action digests equal the earlier staged pilot. Two arms
have identical geometry and *all* action bytes through zero-based t224,
not merely same setup counts. All humans stay connected to service lanes
through their respective bank releases. Explicitly credit only actual
simultaneous legal closures into exactly four connected cells.

| Metric (old dev30) | t225 release | t275 release |
|---|---:|---:|
| Direct exact-four captures | 219 | 248 |
| Simultaneous occupied tube seals | 210 | 238 |
| Direct species cow/pig/rabbit/dog/cat | 23/37/38/77/44 | 26/44/45/79/54 |
| Direct captures per blocks 0–49,50–99,100–149,150–199,200–249,250–274 | 10/68/69/49/23/0 | 10/68/69/49/36/16 |
| Captures originating at t225–274 | 0 | 29 |
| At-release residual → final residual | 216→174 | 187→183 |
| Mean final human union (all components) | 608.10 | 693.33 |
| Mean final actually pet-free human union | 32.53 | 4.90 |
| All-clear cases | 2 | 0 |
| Score sum | 232,815,017 | 157,033,967 |
| Observed maximum wall seconds | .679 | .379 |

Lab scope: paired compact capture wins/ties/losses 18/12/0, score
13/0/17. One can keep catching animals in the late existing bank
topology, at less than one extra compact catch per case on average.
The same fallback under only 25 turns removes four residual pets in
aggregate instead of 42 with 75 turns, negating that gain and leaving
less truly safe area despite more possible human cells. Fewer final
wall actions do not guarantee more clean territory. Reject this longer
delay for score and near-certainty. No new heldout or promotion.

Apply parallel-banks-pilot.patch, parallel-banks-staged.patch and
parallel-banks-release.patch to scratch tactical source, GNU C++23.
Both use CLI3; second argument 225 or275 selects the handoff.

    python parallel_banks_release_bench.py "$HARNESS" "$TESTER" ./tactical ./banks-release "$CASES" compare.csv
