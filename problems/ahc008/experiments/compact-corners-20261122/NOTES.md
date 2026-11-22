# AHC008 shared-guard-tour audit and occupied compact-corner cuts

Independent local experimental workspace. No other contestants' code, solutions, analyses, or future states were used. No remote writes. The comparator `../ahc008_focus_assets/cap4_frozen` SHA256 is `2e47400f50b34407f3fd9aa6c237dc32ffc5f8b93c0207549e12733eb7e30a76`, matching the frozen verified 4×4 capture baseline. Official v3 generator and interactive tester used throughout.

## Shared-tour audit (archived)

Try one or two currently sterile non-refuge connected room frontiers, up to four additional gates, before the primary selected refuge seals. Test exact components on the actual wall grid, verify the virtual primary closure preserves all-clear, require all active humans and the refuge stay reachable after adding the secondary gates, then enumerate an ordered exterior stance/build/retreat tour for one worker. Total construction tour cap 12 worker-actions, return before current evacuation makespan+3, no more than four extra steps for that worker. Diagonal shortcuts are not treated as connectivity. The corrected masks include internal room doors in the two-room interiors, rather than mistakenly trying to close those doors.

On fresh dev30, there were 88 eligible exact primary-closure snapshots. 315 valid motifs and 458 topological single/composite evaluations survived connectivity, 402 survived exterior stance existence; nine orderings satisfied the work-phase budget, zero would return in time. There were no actions/activations. Exact replay gave 29/30 all-clear and 216,223,497 for both solver and frozen baseline. A deliberately generous 24-action, +20-detour audit had two feasible snapshots in the same case, both only single frontiers (best 49 gross cells). An alternative in which one worker remains in a separate pet-free annex, with no need to return, also had zero strict opportunities at the examined timing. Thus topological legality itself was not the principal bottleneck; the narrow interval to staff and traverse cuts was. No official or holdout cases used for this no-op candidate. Archive `*-correct.cpp`, their CSVs and `dev-verified.*`.

## Actual occupied compact corner frontiers

The updated user target is animal confinement inside small components, not merely human evacuation; no all-clear-refuge prerequisite is imposed. The candidate chooses one currently occupied local corner cut from diagonal and short rectangular templates. Each template's required boundary is all still-open neighbors on the actual board, rather than prescribed gates. It uses an exact graph check to ensure the resulting occupied component is at most 32 cells, with no human inside and the main human component intact. It sends at most five nearby workers while retaining non-conflicting baseline actions; briefly missing occupancy is tolerated only in the refined variant and it cannot intentionally place the final wall on an empty pocket. The time-bounded attempt is aborted if the animal leaves the vicinity. `compactroute.cpp` contains diagnostics, `compactroute-clean.cpp` is the frozen no-diagnostic single file. Pet movement is observed causally from the official simulator.

| Policy, same generated dev30 | Score | All-clear | Residual pets | Final compact pets | Mean animal area on all-clear |
| --- | ---: | ---: | ---: | ---: | ---: |
| capture4 frozen | 216,223,497 | 29/30 | 16 | 0 | 682.21 |
| early compact construction | 267,193,550 | 27/30 | 28 | 3 (31, 6, 6 cells) | 661.48 |
| late, current occupation only | 243,338,308 | 29/30 | 14 | 3 (6, 6, 6 cells) | 672.62 |
| late, exterior-only paths and short nearby occupancy slip | 233,416,662 | pending exact topology | — | 5 online exact | — |

Early mean is not paired and its all-clear rate fell. Late’s 24 attempts took 41 proposed wall actions and 71 active turns; 17 ended after pets left, 4 after the deadline, 3 closed occupied. The three successes were a pig, cow, pig on cases 0007 (turn 201), 0015 (171), 0021 (129), in each case completed in five turns from 4–5 initial gaps. Animals began at least two steps from any required wall. Actual input seeds, footprints, events and action sequences are in `auditdev/seeds`, `first30`, and `footprints`. The route refinement was frozen after pilot count 5 vs 3. Development sample was not the archive or any system seed.

## Reproduce

    cd ahc008_tour
    g++ -std=gnu++23 compactroute-clean.cpp -o compactroute-clean
    python ../capture-first-analysis/benchmark_cases.py ../ahc008_focus_assets/tools/target/release/tester compactverify/in compact-hold.csv base=../ahc008_focus_assets/cap4_frozen route=./compactroute-clean --count 100 --capture-topology compact-hold-topo.csv
    python ../capture-first-analysis/benchmark_cases.py ../ahc008_focus_assets/tools/target/release/tester ../ahc008_focus_assets/tools/in compact-practice.csv base=../ahc008_focus_assets/cap4_frozen route=./compactroute-clean --count 100 --capture-topology compact-practice-topo.csv
    python summarize-topology.py compact-topology.csv compact-hold-topo.csv compact-practice-topo.csv

Fresh holdout seed i is the little-endian first 8 SHA256 bytes of `ahc008/tour/202611/compactverify/{i}`. Distributed cases used only after freezing. Exact benchmark wrapper validates legality and replays recorded actions to check the official score and component topology.

## Frozen official validation and decision

| Case block | Policy | Official score | All-clear | Distinct residual pets | Compact pets in final pens | Mean compact pen size | Worst tester time |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Fresh generated 100 | capture4 | 932,151,580 | 99 | 11 | 0 | — | 0.250 s |
| Fresh generated 100 | compact route | 913,030,140 | 95 | 60 | 6 | 6.0 | 0.430 s |
| Distributed 100 | capture4 | 912,125,586 | 98 | 36 | 0 | — | 0.256 s |
| Distributed 100 | compact route | 885,809,109 | 94 | 65 | 15 | 6.6 | 0.366 s |

Zero illegal actions or timeouts on 400 official and independently replayed executions. Compact pens were 3–12 cells on fresh cases and 3–14 on distributed cases. A compact pen has <=32 cells, at least one animal and no humans. Final occupancy is independently certified, not inferred from a proposed cut. Across both boards with success for both solvers, animal-only area fell 0.628 cells on 94 generated cases and 1.522 on 92 distributed cases. Reproduction and case IDs are in the CSVs. Do not adopt as a replacement: the structural mechanism is real but only gained 21 compact individual captures in 200 boards and reduced all-clear by eight. Distinguish the two metrics rather than treating larger clean refuge or transient capture proposals as successful confinement.
