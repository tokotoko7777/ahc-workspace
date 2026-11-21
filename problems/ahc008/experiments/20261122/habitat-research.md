# AHC008 habitat research checkpoint

The target is a strictly greater score than **6,263,333,336 over 100 cases**. This is not a merged/verified rank win. The tester and generator are official v3 from the task page; zip SHA-256 `bee27b0f7a548629c55c4a1c74e9a670264f229fd501f662f401356cab82e490`. The official distributed practice set serves as a distinct comparison, not as proof of matching the online provisional or final test. Original system seeds and participant editorials / solutions were not used in experiments.

## Checkpoint version: parallel-freeze-v1

- `../../main.cpp`: historical adaptive divider baseline (kept unchanged).
- `../../solver.cpp`: modular checkpoint; uses the existing FlatGridFirstStepBfs library.
- `../../submission.cpp`: standalone GNU C++23 source generated with the repository bundler. The no-argument defaults are the frozen config, including the 1.65 CPU-second expensive-search soft limit. No optimization flags needed beyond selecting C++23.
- Dogs are attracted to a corner triangle formed with diagonally adjacent walls. Leave an optional designated person on the pet side while others exit, then close when all legal conditions hold. Candidate corner/size balances weighted travel, passive pet density, and area.
- In the remaining shared area, generate compact diagonal-wall cages around observed pet clusters. Partition mobile workers into at most two groups pursuing spatially separated cages. Assign builds and moves jointly, reserve current and future occupied cells, and preserve connectivity of mobile workers with a proposed-turn BFS rollback.
- Late turns search straight/diagonal seams that attach to existing obstacles. Flood-fill proposed splits to identify the best pet-penalized surviving area, evacuate builders to it, and close when safe.

## Frozen independent generated 100

Official tester + independent transcript replay, rounded official score. Case SHA256 `5f588f2e40b4db021ad65ab63fa19fd687dbf590e8b5d3e06aba61cdf75762d2`. This set was generated independently from the experiment-development seeds after freezing defaults.

| Solver | Total | legal | max direct tester wall | all humans pet-free | mean reachable area / 900 |
|---|---:|---:|---:|---:|---:|
| Adaptive | 280,010,038 | 100/100 | 0.058020 s | 3/100 | 139.97 |
| Habitat v1 | 1,142,886,072 | 100/100 | 0.680647 s | 6/100 | 197.37 |

86 wins, 14 losses. Largest remaining penalty: preserving wide space after removing the final pets. A final cut often achieves a cleaner but much smaller region.

## Reproduction

```sh
tools=/absolute/path/to/extracted/tools
(cd "$tools" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc008/main.cpp -o "$work/adaptive.cpp"
g++ -std=c++23 -O3 "$work/adaptive.cpp" -o "$work/adaptive"
g++ -std=c++23 problems/ahc008/submission.cpp -o "$work/habitat"
python - <<'PY' > "$work/seeds.txt"
from hashlib import sha256
for i in range(100):
    print(int.from_bytes(sha256(f"ahc008/20261122/parallel-freeze-v1/fresh100/{i}".encode()).digest()[:8], "little"))
PY
(cd "$work" && "$tools/target/release/gen" seeds.txt)
python problems/ahc008/benchmark_cases.py "$tools/target/release/tester" "$work/in" "$work/fresh.csv" adaptive="$work/adaptive" habitat="$work/habitat"
python problems/ahc008/benchmark_cases.py "$tools/target/release/tester" "$tools/in" "$work/practice.csv" adaptive="$work/adaptive" habitat="$work/habitat"
```

## Development notes (different seeds, not final comparisons)

- Interior longitudinal cages with narrow access: one side entrance to each short pocket captured too few passive pets. About 0.3–2.0 million average over pilot cases; even preserving area helped little because of the exponential pet multiplier. Accidental full-length dividing version reached about 6.6m but stranded workers, discarded.
- Observation-guided small diamond cages: ~3.9m in a 30-case pilot. Corner dog lure before cages: ~5.6m and two ≥50m cases. Cheap dynamic late seams: 11.3m; adaptive choice of dog corner and size: 13.5m.
- Larger pockets: up to 14.7m; connectivity rollback: up to 16.7m; parallel capture: up to 18.8m. These are exploratory best scores, not an estimate of final expected score.
- Reactive damaged pocket closures and early seams did not improve reliably. Fixed-grid probability propagation for passive animals plus biased dog/cat moves was not better than its corresponding pilot control.
- Next: relax cage shape with min-cut while preserving cohesion; test dog target posterior and tighter exit timing; compare shortlisted policies on fresh held-out generators without using the distributed practice cases for tuning.

## Checkpoint version: mincut-freeze-v2

The fixed-diamond family is still available. For each nearby active animal, generate additional custom perimeters by a vertex-split s–t minimum cut. Source owns a small diamond around the animal; choosing a cutting cell is charged for its build plus travel time, and interior cells incur an area charge. Existing walls and map boundaries naturally provide free edges. A high-scoring irregular pocket can win against all fixed diamonds; the same cohesion guard, assignment logic, two-worker-group policy, and late seams execute the chosen boundaries.

- Modular: `../../mincut_solver.cpp` using existing FlatGridFirstStepBfs and MaxFlow.
- Standalone: `../../mincut_submission.cpp`; use same benchmark invocation as v1 with `g++ -std=c++23` and no runtime flags.
- Both generated sets improved: development100 1,733,637,921 vs parallel 1,510,807,240; separate generated e1 1,530,047,562 vs 1,376,164,470. These are direct tester exploratory comparisons; records below are live official tester plus recorded-action replay.

| 100-case set | Adaptive | Habitat v1 parallel | Habitat v2 mincut | v2 legal | v2 max direct tester wall |
|---|---:|---:|---:|---:|---:|
| Frozen generated | 280,010,038 | 1,142,886,072 | **1,483,627,404** | 100/100 | 1.259053 s |
| Official distributed practice | 259,408,028 | 1,153,430,892 | **1,338,602,186** | 100/100 | 1.179565 s |

Practice set hash `abdbb75810c19e3a410fe7563eca05213af44640ee78bd64db5ef837400f18f1`; no tuning on it. Under the 1.65 s solver CPU soft cap, terminal actions on a few cases can vary with process/runtime conditions; recorded-action replay eliminates ambiguity about what the tested solver actually did. Direct elapsed timings above include the wrapper's tester process and came from mixed workstation load. This is below the 6,263,333,336 goal.

Fixed-wall optimistic cap, excluding all pets and optimally assigning every person to the largest component: 4,380,222,265 (generated100), 4,295,666,711 (practice100). Thus pet prediction alone is insufficient; v2 even with perfect removal and placement needs to preserve substantially larger connected components. In sampled traces, 3–5 animals remained at turn 200; late seams frequently removed roughly half of the remaining 450–650 cells. Large pockets and redundant walls can also split off harmless cells.

Other pilots rejected: deterministic posterior-informed dog decoy selection, delaying the dog phase to focus on nearby passive animals first, building from the pocket interior, and simulated random-walk capture probability estimated from static job routes. None improved score consistently over v2 in the 30-case development pilots.
