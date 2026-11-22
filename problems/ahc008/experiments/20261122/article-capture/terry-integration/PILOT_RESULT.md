# TERRY integration: frozen learned-map pilot, not adopted

The integrated single-file GNU C++23 solver in this directory implements
the article-based construction, real lane handoff, forecast movement and
capture coordination, lane connectivity protection, distinct standby
assignment, and late optional min-cut with stochastic switching. Refer to
[the untrained integration README](README.md) for scope and deviations.
The five templates adapt different-width source passages to the user's
required exact 1×4 traps; no original submission was used.

Pilot trained on 80 independent official-generated inputs and validated
the model on a separate 20. Each input produced five exact official
labels. Model mean on the 20 was 6,997,117 versus fixed light map
6,861,432 points, a +135,686 difference with stderr 470,974. This did not
meet the pilot publication gate; the exploratory frozen model was then
studied without changing it on the two 100-case cohorts below.

| Exact v7 → integrated TERRY pilot | Official distributed100 | Independent generated100 |
|---|---:|---:|
| All animals separated from humans | 36 → 3 | 44 → 4 |
| All separated and ≥300 mean human cells | 25 → 3 | 26 → 4 |
| Score | 2,655,369,894 → 989,470,357 | 2,688,102,365 → 846,529,625 |
| Final human-accessible animals | 165 → 612 | 139 → 626 |
| Genuine four-cell captures | 822 → 874 | 837 → 907 |
| Genuine four-cell captures by turn 100 | 433 → 101 | 456 → 114 |
| Mean usable cells per person | 434.4 → 639.1 | 411.7 → 643.8 |
| Largest local official tester wall time | 0.990 → 1.049 s | 0.845 → 1.151 s |
| Cases with paired late evaluation; chosen cut; actual cut closures | 67; 7; 0 | 76; 18; 3 |

All 400 exact executions passed the official tester and independent
replay; all 200 v7 inputs, action hashes and scores match history. Do not
conflate the two independent local collections with the contest input.
Early capture collapsed even though individual final 1×4 catches and
available human area rose; the new code is not substituted for v7.
Selected late cut actions were frequently preparation without a seal.
The experiments use an explicit reserve for each trial; median completed
trials per activated case can be derived from the ledger.

Reproduce: compile `solve-terry-pilot.cpp` with
`g++ -std=c++23 solve-terry-pilot.cpp -o solve-terry-pilot`. No flags for
normal play. Use `bench.py` and the same local harness with the historical
v7 and each frozen input cohort. The zipped case ledgers include precise
hashes, status, and per-stage telemetry. Model training uses `train.py`
on directories containing the immutable `manifest.json` and `labels.csv`
with matching split; `collect.py` and the seed namespaces reproduce each.
SHA256 of exact pilot source: `4256212bd37b2d0cd0a0434bc87d4661e7c6940c9446e0524a36f8a9cb04d0f9`.
SHA256 of frozen weights: `8a2ec3d66e45a2fff5470f72114150880de0cd3afd139794cbbe017e397bac1d`.

Next controlled trial retains the original v7 board but leaves all the
other TERRY runtime methods as they were, to
separate dense geometry from scheduling and motion policy.
