# AHC008: a sliding four-cell cage on a thin edge rail

**Mechanism verified, not adopted.** Actual legal construction and two guards can catch a passive animal in four cells, but most cases never yielded a staffed arrival. Sliding the gate positions gave one extra catch over the matched fixed-window control in 60 cases.

## Geometry and action chronology

Construct one 10-tile lateral rail wall along the edge. The boundary supplies the other side and both ends remain open. Along the playable lane index the ten tiles 0–9 and exterior stances -1 and 10. Choose any gate pair \(g,g+5\), where \(g=0,\ldots,4\). Guards stand at \(g-1,g+6\); the resulting connected pen consists of \(g+1,\ldots,g+4\). A pet may safely be at \(g+2\) or \(g+3\), allowing both checkerboard parities. Only close if a cow, pig, or rabbit is observed in a safe interior cell, no pet is on or adjacent to either gate, no human occupies a gate or interior, and the two real guards are simultaneously on their exterior stances.

1. Choose one geometry from visible initial animal positions/types and humans, balancing nearby passive animals with travel and construction distance. Freeze it before simulating either arm.
2. Assign legal wall jobs to available humans. Once all ten exist, choose two distinct guards from the current board by minimizing the larger of their BFS travel distances to the outer stances. Other humans leave the rail if any were on it during construction and otherwise wait. No grouping to lure dogs: only passive cattle, pigs and rabbits guide the experiment.
3. Wait with those two guards physically present until a passive animal is first seen on the rail, or until the fixed 100-turn preparation/wait budget ends. The two arms assert identical preparation, stage times, gate-window choice and an anchor snapshot hash of all observed animal/human positions.
4. From that first staffed observation, allow six turn transitions. The fixed arm chooses the best single translation *at the very same observation*, using the exact per-species finite-horizon model, then walks to and closes only that translation. The adaptive arm can revise next guard steps by the current observed positions and remaining horizon. Both sum model capture probabilities for animals visibly on the rail; actual closure still checks the complete board and all animals.
5. Stop after one closure or this chase horizon. The unmodified official simulator executes every action. After each reported close, flood fill asserts the interior contains exactly four reachable cells and no human and that counted animals remain inside after the official pet movement.

The simulator needs the official seed to produce pet moves; the geometry, placement, movement and closure logic read only visible coordinates/types, current walls and actions. They never access the RNG state or internal dog/cat targets. This is a capture experiment, **not** a 300-turn solver or score comparison.

## Model and matched official simulation

The included `sliding_four_model.py` is the conditional single-animal 1D reference. It uses cow/pig/rabbit 1/2/3 simple steps and treats escape from the rail as absorbing, omitting preparation, arrival, other animals, and legality conflicts. At 10 tiles and six turns its adaptive/fixed averages over uniform starting positions are cow **.64375/.52500**, pig **.55488/.44092**, rabbit **.47978/.40231**. Length six has only one translation. The Rust probe uses the same model to choose guard moves but measures actual official dynamics; it does not treat its analytic probabilities as observed capture rates.

| Arm, 60 new generator cases | Cases with first staffed arrival | Cases closed | Cow / pig / rabbit captured | Other captured |
| --- | ---: | ---: | ---: | ---: |
| 10 walls actually built, fixed translation | 28 | 10 | 4 / 2 / 4 | 0 |
| Same actual prep, adaptive translation | 28 | 11 | 5 / 2 / 4 | 0 |
| Synthetic prebuilt rail, actual guard walk, fixed | 25 | 11 | 4 / 6 / 2 | 0 |
| Same prebuilt prep, adaptive | 25 | 13 | 5 / 6 / 3 | 0 |

All 60 actual cases built and staffed their rail. Mean turns: wall phase **15.95** (median 15), guard staging **6.17** (median 6), staffed waiting before first arrival/cutoff **50.85** (median 70). There were 54 distinct passive animal sightings summed per case in the pre-staff rail, but only 29 at the first staffed observation, in 28 cases. Arrival cases reached that observation at mean turn 43.07 (1-based). Upper bound on operative phase is 100 preparatory/wait turns and six chase transitions plus the closing turn. Actual fixed/adaptive total builds 620/622; human moves 4,748/4,762; waits 27,741/27,682. There was one paired closing gain and no losses in the real test (an additional cow). This effect is too small to support more variants or a score rollout without a new mechanism for getting passive animals to enter sooner.

The prebuilt arms omit the wall placement and prevent early movements through that perimeter. Their guard walk is real but that setup is infeasible as normal game start, and its earlier wall state changes arrival trajectories. Its counts are **not** a matched estimate of the effect of construction time. The fixed/adaptive comparison is matched within each physical setup.

## Reproduce

- Official [AHC008 tools v3](https://atcoder.jp/contests/ahc008/tasks/ahc008_a).
- Seed list `seeds.txt`: 60 u64 values from Python MT key 59634072336; no distributed or final cases. Run official generator here to recreate `in/`.
- Copy `probe.rs` into a temporary copy of the official crate as `src/bin/sliding_rail_probe.rs` and run `cargo build --release --bin sliding_rail_probe`, then `/path/to/sliding_rail_probe in 100 prebuilt > pilot-repro.csv`. The last argument enables the separate counterfactual.
- `build.sh` is the original relative-layout direct `rustc` build recipe; the Cargo route above is portable. `python sliding_four_model.py` prints the conditional reference table.

`pilot.csv` preserves per-case matched anchor hashes, phase budgets, sightings, chosen fixed translation, exact capture species and close timing. No raw move transcripts, generated inputs or binaries are included.
