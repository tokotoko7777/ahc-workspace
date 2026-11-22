# AHC008 same-animal credit ablation (v13 on v12)

This is a research variant, not the recommended solver. Exact v12 parent SHA256
`20d714a57806d97f8aba2371187bf205dde176c3b21ac96873189067d599374d`.
Candidate standalone SHA256
`678139f5792230589ef1d02c9b321caffc3522455b3f0e3e05befa117666b692`.

The existing probability model estimates each animal's ordered position at the
same causal worker-pair arrival time. It does not distinguish two proposed pairs
whose weighted forecast scores mostly describe the same animal. Retain exact
instantaneous legal locks, routing, pair arrival utility, manufacturing and
late-play decisions; diversify only forecast selection. Compute per-animal
weights from conditional occupancy given no blocked gate. With equal remaining
pair count, maximize 40% total pair utility plus 60% per-animal maximum
utility, using up to two single-job sweeps starting at the exact worker-mask DP
answer. Worker and tube uniqueness remain hard constraints. This is a utility
proxy, not a claim that capture events are independent or that overlapping
coverage necessarily has zero value.

Use GNU C++23; no compiler optimization flags required beyond source pragma.
The optional telemetry reports multi-pair plans, replaced plans, and duplicate
weight before and after replacement on each variant trajectory.

```sh
g++ -std=c++23 solve.cpp -o solve
g++ -std=c++23 unit.cpp -o unit && ./unit
export PYTHONPATH="$harness/experiments/20261122:$harness"
python bench.py "$harness" "$official_tester" "$v12_executable" ./solve "$dev30_input" dev30.csv
python summarize.py dev30.csv "$v12_exact_ledger" > dev30-summary.json
```

Same 30 development inputs, sequential official tester and same input/actions
on the official playback plus independent score exact agreement for all 60
runs. All 30 baseline actions and scores match the archived v12 result. The
fixture confirms switch from a duplicated animal to a distinct animal, worker
uniqueness, and single-job no-op.

| Metric | v12 | diversified v13 |
|---|---:|---:|
| all animals isolated | 15 | 17 |
| all isolated with mean human floor ≥300 | 13 | 10 |
| remaining accessible animals | 54 | 42 |
| genuine final four-cell captures | 262 | 255 |
| score | 1,028,811,010 | 890,474,498 |
| median tester wall clock | 0.694 s | 0.704 s |
| maximum tester wall clock | 1.016 s | 1.038 s |

80 of 1,545 multi-pair plans changed (same-state duplicate weight 2.56649 →
2.32160, about 9.5% reduction). Three gained all-clear cases retained only
60–126 human cells and the number of wide clears fell. Do not adopt this
replacement or use the holdouts for retuning it. Next use a short arrival-window
objective rather than discounting the value of a well-timed backup; measure
both real true-four catches and distinct targets reached, together with floor.
No historical ranking inference.
