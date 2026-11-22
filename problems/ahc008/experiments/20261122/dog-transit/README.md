# AHC008: dog capture in a four-cell edge corridor

## Geometry and actual action policy

Use a board edge as one wall of a one-cell-wide passage. Six legal wall builds parallel to the edge leave six open lane squares (two gate squares, four interior). Two external stances, one tile beyond each gate. The open bypass through the global human component is four moves longer between these stances, so a dog pursuing the far-side humans from the near side has reason to cross. Both doors are built together only while at least one dog is in the interior (necessarily in the two central squares if both gates are legal), every human is outside, and no pet is on or orthogonally neighboring either door. The animal cannot follow a human outside after closure, and no human is sacrificed. All uncaptured humans remain in the same surrounding connected component in standalone runs.

The chooser scans 168 directed geometries based only on initial visible animal positions/types and human positions. It rejects occupants in planned wall squares and favors likely near-side dogs, nearer construction, and nearer far bait. Workers greedily assign unfinished wall jobs with BFS distances and explicit simultaneous legality. After six walls are ready, choose a near guard by current relative BFS arrival; walk all other humans to the far guard square as bait. The first legal positive dog cage closes. Fixed attempt cutoff: 75 turns including building, walking and guarding. The standalone proof legally emits waits for the remainder of the 300-turn protocol, not a score strategy.

The contestant implementation reads the actual interactive initial input (positions/types only) and public move strings; its wall state is reconstructed from its own output. It does not see or derive the official generator seed, hidden RNG or hidden active dog targets. No probing or lookahead into future moves. The Rust causal harness reads seed only through `Sim::new(input)` to drive the unmodified official environment. Controller observes coordinates; it never reads the dynamic pet enum's target field. The exact decoded first-substep target posterior was **telemetry only** in the unchanged constructive controller. A trial fixed re-bait turn did not improve the pilot.

The prior `tunnelguard.cpp` organized many broad strip compartments with passive occupancy/near-pet gate guarding. This has one four-cell local capacity, explicit near/far bait, and both doors staffed by actual exterior humans. It does not assume prebuilt walls in the real arm.

## Fresh causal pilot (official compiled v3 Sim)

75/80 cases had dogs, 237 initial dogs. Each arm uses the same visible geometry choice and sim input (same seed), stops after first closure or 75 turns. Counterfactual prebuilt arms are explicitly not legal game-start claims. No illegal transitions in any constructive run.

| Arm | Cases with dog caught | Dogs / other caught | Sum wall turns | Turns two exterior guards present | Fully staffed far-bait turns |
| --- | ---: | ---: | ---: | ---: | ---: |
| Actual legal construction | 68/75 | 76 / 0 | 974 | 1459 | 626 |
| Side walls prebuilt, humans walk from true initial positions | 73/75 | 91 / 1 | 0 | 1632 | 665 |
| Side walls prebuilt and staff teleported to stances | 75/75 | 79 / 0 | 0 | 983 | 983 |
| Actual with re-bait after 24 fully staffed turns | 68/75 | 76 / 0 | 974 | 1485 | 592 |

Don't compare staff turn totals as exposure-controlled rates: arms end on first positive closure, and prebuilding/staffing changes realized pet movement. Constructive pilot observed 272 dog-turns near the near approach while guards were present; 251 had a shortest route through the first gate to the far bait, and 86/272 had >0.7 inferred posterior mass on far-side bait targets (observed move inference, not an oracle). These are repeated dog-turns, not independent chances or proof every far dog enters.

## Fresh full interactive verification (C++23, official tester)

On a separate 80 generator cases, all outputs had 300 legal turns with the original official tester. Max measured tester+solver wall time 0.074 s. Cases with dogs: 77; cases with dog capture: 65; captured **91/266 dogs**, all in a real four-cell pet-only cell group, and 0 incidental other animals. Remaining: **175 dogs and all 910 other animals** in the shared human component immediately and at turn 300. Safety/equivalence: case-level C++ capture counts matched all 80 constructive Rust pilot cases exactly.

For 77 dog cases, attempt lasted mean 47.5, median 44 turns (mean 42.45 among successes); side-wall phase mean 13.42, median 13; both guards present mean 23.29, fully stacked bait mean 12.49. Human actions during the attempt: 592 builds (mean 7.69, exactly eight in successful cases), 13,335 walking moves (mean 173.18), 13,001 waits (mean 168.84). The three cases without dogs trigger no attempt. This is an expensive upfront action commitment; standalone captures are not a score result.

A separate repeated-capture proof reuses the existing wall state, masks already isolated dogs via component membership, and starts a second fresh cage only after success, if at least 25 turns remain and a reachable dog is still at large. Same absolute cutoff of 75. In the same 80, isolated dogs 91→129 and 1 incidental other pet. 42 cases restarted, 31 gained at least one dog; extra costs totaled 1,277 elapsed turns (16.6 per dog case), 313 builds, 3,788 walking moves, and 5,181 waits. No illegal outputs. This is also not a score result.

## Integration experiment

`transit_cap4.cpp` fronts unmodified cap4 residual policy with the isolated single-hunt module. The real global turn and current wall/position states are retained; no fake reset or idle score comparator. First40 (exploratory, not a new confirmation of the standalone success rate): cap4 340,001,275; single hunt cut off at 75, 391,870,386 (+15.26%, 16 wins/7 ties/17 losses; wide paired-bootstrap interval about -11.8% to +52.2%); 30-turn limit 315,429,716 and caught only 3 dogs. The last40 paired benchmark uses official tester plus independent score replay and topology sidecar.

## Reproduce

Working directory: this directory. Official gen and tester were existing release v3 artifacts, not modified. No editorial, competitor solution, distributed test IDs, hidden contest cases, submissions or network.

    bash build.sh
    ../../ahc008_focus_assets/tools/target/release/gen seeds.txt
    ./probe in 75 0 1 2 3 > pilot-repro.csv
    g++ -std=c++23 standalone.cpp -o standalone
    (cd confirm && ../../../ahc008_focus_assets/tools/target/release/gen seeds.txt)
    python test_wrapper.py confirm/in official80-repro.csv
    g++ -std=c++23 repeat.cpp -o repeat
    python test_repeat.py confirm/in repeat80-repro.csv
    python make_integration.py
    g++ -std=c++23 cap4_original.cpp -o cap4_original
    g++ -std=c++23 transit_cap4.cpp -o transit_cap4
    python test_pair.py confirm/in score40-repro.csv 40 base 75 30
    python ../../ahc008_focus_solver/problems/ahc008/benchmark_cases.py ../../ahc008_focus_assets/tools/target/release/tester confirm-tail replay40-repro.csv base=./cap4_original edge=./transit_cap4 --count 40 --timeout 3.0 --capture-topology replay40-topology-repro.csv

Seed lists from independent Python MT seeds 2249044512 (pilot) and 1152398423 (C++ verification). `confirm-tail` holds symlinks to cases 40–79 of verification. `transit.hpp` is the reusable helper and self-contained integration inlines it. All new work is isolated locally.

### 40-case exact replay and pre-frozen score gate

The untouched last40 of the 80-case verification were replayed using official tester plus independent transcript score validation. Cap4 266,666,901 vs single edge/cap4 340,888,886 (+27.83%, 21/6/13); 39/40 all-clear each, no failures. Mean clean area per clean human 62.24→77.49; mean distinct residual pets 0.425→0.025. The frozen source and binary hashes in `frozen.sha256` were fixed before independent fresh100 and distributed100.

Fresh frozen100 reversed: cap4 **875,592,712**, 99 all-clear, versus edge/cap4 **849,152,563**, 97 all-clear (33/20/47). Both 0 invalid outputs, every score replay-matched. Max wall 0.262/0.377 s. Topology means: distinct clean area 78.89→76.42, mixed area 7.01→23.16, distinct residual 0.17→0.47, distinct pet-only area 662.91→649.50, largest animal-only group 660.01→640.55. Do not promote standalone prefix as a general score policy. It still establishes actual compact dog confinement at substantial action cost.

The bottleneck is not just six walls: across 77 dog cases, mean 13.42 wall-phase turns, then 10.82 more before two exterior guards ready, then 23.29 dog-wait/closure turns. Those waits account for 13,001 in-prefix unit-action idles. We therefore separately try concurrent baseline scaffold work after two guards arrive, keeping two far-side bait humans plus near guard and releasing only additional humans already at far stance. It runs the reference scaffold's `roomBuild` and priority on eligible humans with a protected radius-two buffer around the trap. The untouched CAP4 closing/late policy resumes once the attempt ends. This is not a tuning of the attempt cutoff.

Distributed100 independently replayed: cap4 **912,125,586**, 98 all-clear, versus edge/cap4 **897,546,719**, 94 all-clear (37/21/42). Both produced 0 invalid outputs; max tester time 0.257 / 0.376 s. Mean distinct safe area 82.10→80.70, mixed 14.97→38.22, residual 0.36→0.80, pet-only area 651.67→634.79, largest pet-only area 645.11→624.87. Both freeze benchmarks weigh against the score overlay. The compact confinement geometry is retained, not the proposed default integrated policy.

On the original fresh40 development split, simultaneous scaffold added **782 actual builds** and **3,243 walking moves** by released humans during the prefix, with no illegal outputs. However, capture fell **45→27 dogs** (count improves/ties/worsens in 5/21/14 cases), total prefix 1,932→2,026 turns with identical initial wall-phase total 508, and full score **391,870,386→334,481,084** against untouched baseline 340,001,275. This gains real scaffold progress but compromises the dog-only subject. No score holdout or adoption. `walks`/`waits` for the transit module in overlap CSV are its requests before scaffold substitutions (not all actually executed); `scaffold_builds` and `scaffold_moves` are post-merge executable assignments.

### Exact posterior advisory release

Naive release can move away a pursued target. `transit_informed.cpp` estimates each moving dog's target from observed first and second steps with the existing exact `AHC008DogTargetFilter`, not the simulator state. Release only if (a) both sides already staffed, (b) a near dog appears to use the first gate as a shortest route and has >.70 far-bait posterior, (c) the worker has <.09 weighted target risk across reachable dogs, and (d) two far humans remain. Same late policy, same 75-turn cap, no change to the direct cage closure condition.

On the first40, this released workers in only 5 cases, saved 45 scaffold builds and produced 234 scaffold walks, captured 49 dogs vs 45 for serial (+4 across three differing cases), and scored 376,555,555 vs 391,870,386 for serial. On the separate last40 release fired in 3 cases, delivered 25 builds/148 walks, captured the identical dog count in every case (46 total), and scored 347,686,764 vs 340,888,886 for serial. Actual progress rare, no validation of clear-rate/material gain, no adoption.

Replay commands:
    python make_overlap.py
    g++ -std=c++23 transit_overlap.cpp -o transit_overlap
    python test_overlap.py confirm/in overlap40-repro.csv 40 base 75 overlap
    python make_informed.py
    g++ -std=c++23 transit_informed.cpp -o transit_informed
    python test_informed.py confirm/in informed40-repro.csv 40 75 overlap
    python test_informed.py confirm-tail informed-tail40-repro.csv 40 75 overlap

`standalone_bundled.cpp` and `repeat_bundled.cpp` are GNU C++23 single-file copies of the POC controllers. The preserved practical inference is that active live bait can place actual dogs in four-cell pens without human capture, but using most workers for the bait costs enough construction time to reverse score/clear-rate gains at scale.

### Reproduce from this repository archive

Use the official [AHC008 tools v3 linked on the task page](https://atcoder.jp/contests/ahc008/tasks/ahc008_a). The original commands above name the isolated experiment workspace; reproduce that layout in a temporary directory without changing your checkout:

1. Put or symlink the official tools directory at `$TMP/ahc008_focus_assets/tools` and build the release binaries with Cargo. Symlink this repository at `$TMP/ahc008_focus_solver`. Copy the files in this archive to `$TMP/work/dog-transit` and use that as the working directory.
2. For the causal Rust probe, copy `probe.rs` to the **temporary official tool source** as `src/bin/dog_transit_probe.rs` and run `cargo build --release --bin dog_transit_probe`. Use the resulting `dog_transit_probe` as `./probe` instead of the local-layout `build.sh`. It links the same official `tools` simulator library without modifying its transitions.
3. The three seed lists are `seeds.txt`, `confirm/seeds.txt`, and `fresh/seeds.txt`. Run the official generator in each of those directories to recreate their individual `in/`. For the exact tail replay, create `confirm-tail/` and link inputs 0040–0079 from `confirm/in/`; then run the benchmark above. Keep the official distributed `tools/in/` intact; use it only for the final frozen distributed comparison.
4. Recreate the frozen paired benchmarks by replacing `confirm-tail` in the benchmark command with `fresh/in` or `../../ahc008_focus_assets/tools/in`, selecting a different output filename and topology filename, and using `--count 100`. The repository benchmark harness performs the independent transcript replay. Case CSVs and certificate JSONs are retained here; raw move streams, binaries, and generated inputs are omitted.
