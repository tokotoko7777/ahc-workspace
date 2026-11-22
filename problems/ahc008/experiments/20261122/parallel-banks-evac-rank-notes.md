# Before-commit crew evacuation ETA in the existing wide-line ranking

The independent audit found recurrent crew ETA underestimation. This edit
leaves every line, area threshold, wall risk estimate, crew movement,
and ongoing policy intact. For an already-qualifying wide separator,
obtain the actual retained component that the unchanged executor will
choose (unpenalized score, rather than using the may-differ projected
risk-adjusted side). For current mobile humans in that component's
original group, find exact current-grid shortest distance to any retained
cell. Their maximum is a crew lower bound, ignoring further delays and
future reservations. Pay max(old builder ETA, crew ETA) in the existing
ranking denominator and reject only deadline-infeasible candidates.
Does not model varying pet paths, change max thresholds, recompute old
risk, alter act execution, or add candidate geometry. Per-group/time
distance bit-row prefixes avoid repetition. Same frozen narrow rescue
default; compare flags `5 1 1 0` vs `5 1 1 1`.

| Used frozen dev30 | Narrow | Exact crew ETA |
|---|---:|---:|
| Exact official score | 746775771 | 721422786 |
| Fully clear cases | 18 | 12 |
| Final residuals | 42 | 69 |
| Pet-free people | 136 | 93 |
| Distinct safe area sum | 5884 | 5177 |
| Direct bank exact-four captures | 206 | 206 |
| Final exact-four animals | 212 | 213 |
| Post-release nonrectangular 4–48 captures | 53 | 51 |
| Median/max seconds | .346/.478 | .380/.565 |

W/T/L 2/17/11, entire outputs identical in 17. All 60 official
tests and independent exact replay pass, unchanged control hashes,
t0..release exclusive byte-identical for every pair, same handoff
swap decisions; a 3-case smoke preceded the benchmark. Across the 30
cases the new check was applied to 38,202 candidate evaluations, raised
ETA for 7,367 finite candidates, rejected 4,434 for impossible deadline,
and ultimately 32 committed lines paid a higher ETA. Counts include
repeated searches; they are not unique geometries. A tight current
distance lower bound is valid, but using it to prefer seemingly easier
cuts did not improve actual retention or capture on this dev sample.
No holdout, tuning grid, promotion or main.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-evac-rank.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-evac-rank.patch"
g++ -std=c++23 /tmp/ahc008-evac-rank.cpp -o /tmp/ahc008-evac-rank
python "$D/parallel_banks_evac_rank_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-evac-rank "$DEV30" /tmp/evac-rank.csv --frozen-control-csv "$D/parallel-banks-empty-guard-dev30.csv"
```
