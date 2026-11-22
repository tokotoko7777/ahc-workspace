# Persistent exact dog target filter: animal-side audit

All 30 narrow-rescue action hashes and scores match, with official and
independent recorded replay. Use the already-existing
`problems/ahc008/dog-target-filter.hpp` from branch
`codex/ahc008-benchmark-20261122`; no reimplementation of posterior
updates. SHA256 `e7e692774601d762ca08de7e3d604a6a0e77d4d5ed813cc29d9e54d519f2a287`.
At each actual **post-human** wall/human graph, condition a dog's latent
target estimate on only previous observed first/second steps. This is
an audit, not an actual player decision or access to hidden targets.
Compare persistent posterior to uniform reset and directional
first-step heuristic (×1.6 repeat, ×0.5 reverse), both using legal
shortest-neighbor support. Exact path prediction from current graph
for 1–3 turns uses future fixed humans/walls; observed controls use
actual later human and pet responses and are not model training.

| Label | Observations | Reset | Direction | Posterior |
|---|---:|---:|---:|---:|
| Next actual directed-step log loss | 10903 | .991 | .828 | .430 |
| t101–handoff directed-step log loss | 1736 | 1.040 | .827 | .383 |
| Geometrically safe exact-four visit, Brier | 994 | .1914 | .1600 | .1089 |
| Actual any1–3 all-pet legal, Brier | 994 | .1755 | .1531 | .1314 |
| Actual all-pet legal **and staffed**, Brier | 994 | .1556 | .1443 | .1605 |

Only prepared, presently not globally legal tubes with an actual dog
on the body/gate/exterior stance and distinct-worker stance ETA<=2.
Post101 sample has 415 windows and 103 actual legal positives; legal
Brier .1833/.1613/.1423 (posterior better in 15/20 cases than heading).
Staffed Brier .1659/.1548/.1723 on the same rows. Thus the model
clearly improves animal-side prediction (directed-step better in all
26 with observed directed steps), but not supplied crew availability.
No live policy and no cats in this audit. Exact per-event records
distinguish physical safe occupancy, all-body legal and staffed outcomes.

```sh
D=problems/ahc008/experiments/20261122
g++ -std=c++23 -I"$BENCHMARK_CHECKOUT/problems/ahc008" "$D/parallel_banks_dog_posterior_probe.cpp" -o /tmp/dog-probe
python "$D/parallel_banks_dog_posterior_bench.py" "$HARNESS" "$TESTER" "$NARROW" /tmp/dog-probe "$DEV30" "$D/parallel-banks-empty-guard-dev30.csv" /tmp/dog.jsonl /tmp/dog-summary.json
```
