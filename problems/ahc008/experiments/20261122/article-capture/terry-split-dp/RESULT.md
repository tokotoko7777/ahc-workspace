# Exact fast two-orientation construction split

The new DP is observationally equivalent to the accepted exact-short
solver at fixed completed rollout counts. Profile found initial build
split accounted for 55.3% of measured no-rollout stages. Replace only
that function with flat mask×prefix tables, compact predecessors,
precomputed entry distances, exact segment-coverage bounds, and one
cost comparison per orientation pair. Map, human path, wall commands,
all simulation, RNG, and tie policy are unchanged. 360 orientation
fixtures assert exact queues and makespan, including all five maps,
all official worker counts, concurrent-start ties, and small random
inputs.

| Fixed workload | Cases | Control wait4 CPU | Fast wait4 CPU | Saved |
|---|---:|---:|---:|---:|
| Official distributed | 100 | 69.676003 s | 48.666386 s | **30.1533%** |
| Prior independent held | 100 | 72.364498 s | 53.603679 s | **25.9254%** |
| Distributed, positive rollouts | 12 | 31.324845 s | 28.151142 s | 10.1316% |
| Independent, positive rollouts | 14 | 36.829423 s | 33.671545 s | 8.5743% |
| Distributed, no rollouts | 88 | 38.351158 s | 20.515244 s | 46.5068% |
| Independent, no rollouts | 86 | 35.535075 s | 19.932134 s | 43.9086% |

All 400 official tester invocations reproduce the complete frozen
300-command stream and associated score. Both totals clear the 2%
predeclared threshold. Max local tester wall is 2.7044→2.6936s
distributed, 2.7518→2.6547s independent. These are serial, alternating
order, fixed-work profiles, not an end-to-end score claim; usual clock
follow-up recorded separately. No per-instance cap in the live solver.
C++23 source SHA-256
`170382b80e4ca1b0d3a5c4f0948527edabfbb43c0549fa8512fbe9dccc59e223`.

```sh
python ahc008_terry_split_dp/make.py
g++ -std=gnu++23 -O2 ahc008_terry_split_dp/unit.cpp -o /tmp/terry-queue-unit
/tmp/terry-queue-unit
g++ -std=gnu++23 ahc008_terry_split_dp/solve.private.cpp -o ahc008_terry_split_dp/solve
python ahc008_terry_speed/bench.py TESTER INPUT_DIR EXACT_H123_LEDGER ACCEPTED FAST result.csv
```

Use the accepted exact-short h123 private C++23 source (pinned by the
transform); the fixed-work harness and its wait4 helper come from the
earlier speed experiments. Archive consists of independent transform
and tests, frozen measurement logs, and default follow-up protocol,
not private source or binaries. See the companion reuse proposal.
