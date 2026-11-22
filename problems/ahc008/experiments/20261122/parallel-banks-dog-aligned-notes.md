# Staffed one-turn dog-posterior opportunity (read only)

Frozen narrow dev30 with exact complete score/action hashes and official
recorded plus independent replay. Starting t101, use actual normal
rosters from the unchanged controller. Require both assigned guards in
their actual lanes, on/in one step of their own stances and at least one
already in position, prepared open exact-four, dog already on its mouth,
body or actual guard stance, no already-legal closure of this target,
no competing currently legal staffed cage and no ordinary wall command
for the guards. No new worker, purposeful lure, rover, or other relocation.
Separate whether hold/align would actually change a normal command and
whether it would consume another normal selected pair assignment, including
a guard genuinely reserved while already holding.

Use exact conditioned latent dog posterior from the prior audit, forecast
one turn with the *hypothetical* two held/aligned human endpoints and
observed other human moves/walls. Classify factual future dog-safe position,
other-pet blocks and actual all-pet-legal closure opportunity separately.
Observed future states are not model inputs or counterfactual proof.
Baseline-final-unresolved pets are only a retrospective coverage label.

| Sample | Rows | Cases | Other selected pair | Factual next legal | Final-missed dogs with factual legal chance |
|---|---:|---:|---:|---:|---:|
| Normal pair at/within one | 216 | 20 | 31 | 29 | 1 |
| Would genuinely change the guards' commands | 30 | 12 | 30 | 3 | 1 |
| Changed, one-turn dog-safe p>=.50 | 2 | 2 | 2 | 1 | 0 |
| Above, no existing other pair consumed | 0 | 0 | 0 | 0 | 0 |

30 of 59 factual dog-safe endpoints in the first set had some other
animal blocking closure. The recalculated held and factual dog one-turn
probabilities agree to logged precision in these close alignment scenes;
this is measured, not an assumption that the animal path remains factual.
The predeclared >=6 distinct otherwise-final-missed dog IDs over >=5 cases
before considering a live arm was not reached. No live solver, cats, new
cohort or promotion.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/align-observer.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-dog-aligned-observer.patch"
patch -s -o /tmp/align-probe.cpp "$D/parallel_banks_dog_posterior_probe.cpp" "$D/parallel-banks-dog-aligned-probe.patch"
g++ -std=c++23 /tmp/align-observer.cpp -o /tmp/align-observer
g++ -std=c++23 -I"$BENCHMARK_CHECKOUT/problems/ahc008" /tmp/align-probe.cpp -o /tmp/align-probe
python "$D/parallel_banks_dog_aligned_bench.py" "$HARNESS" "$TESTER" /tmp/align-observer /tmp/align-probe "$DEV30" "$D/parallel-banks-empty-guard-dev30.csv" /tmp/dog-align.jsonl /tmp/dog-align-summary.json
```
