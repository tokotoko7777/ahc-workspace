# Deliberately staffing two adjacent 1×4 traps from one pair of exterior stations

Pinned v25-fast2 source commit d6eafd3ff9edb8f55421e1df903113d30efbd1ce. The geometry and existing brace work stay fixed. In each of the six opening layouts the topology fixture finds 24 shared two-person stations, each able to close either of two neighboring 1×4 traps without moving or closing the three aisles. A shared role is added only after the original constructor deadline and only with five humans. The stock immediate aligned closure keeps priority. Forecast proposal scores the better legal catch in each same trial, not the sum, and may replace a stock axial pair when its packing score is strictly better. See source README, geometry/legality tests and exact generator in the archive.

## Paired official development result: not adopted

Exact same M5 30 historical inputs, official tester and separately checked score/legality replay for both candidates. All 60 runs legal.

| Metric | v25-fast2 | shared station |
|---|---:|---:|
| Exact four captures by t100 | 68 | 73 |
| Exact four captures by t160 | 191 | 190 |
| Accessible animals at end | 79 | 93 |
| All caught | 11/30 | 8/30 |
| All caught, mean human area ≥300 | 4/30 | 1/30 |
| Official total | 536,821,892 | 360,354,887 |
| Max local tester wall time | 0.560 s | 0.664 s |

The alternate packed 2,126 jobs over 1,963 turns; candidate telemetry reports 63 same-turn alternate closures for 65 heads across 27 cases. This tests a real mechanism, not merely incidental station occupation. Score improved on 13 boards and declined on 17. One-by-one identity comparison finds 32 candidate-only exact-four catches by t100, but 26 were already caught in exact four by stock t160 and 29 were isolated by stock by the end (any capture shape). At t160 still accessible by species, candidate minus stock: pig −13, dog −5, rabbit +2, cat +5, cow +12. At final: pig +5, dog −2, rabbit 0, cat +3, cow +8. These are paired behavior counts and do not prove which modified first action causally created a specific loss. Score/runtime totals reflect the full policy and not per-job time.

No held cases opened. Retain v25-fast2. Next diagnostic: can shared stations add meaningful coverage with an unused crew without displacing stock pairs, routes or later paid work?

## Reproduction

Extract `evidence.tar.gz` into the workspace root. It includes
`ahc008_v25_midpoint_live/{make.py,live.inc,unit.cpp,README.md}`, both fixture
sources in `ahc008_v25_midpoint_certificate/` and the development data and runner
in `ahc008_v25_midpoint_bench/`. Generated single-file candidate
SHA-256 `d0be9b6f75ab34ea34009e984485d510393e887afb74514d492884d1a18d06b2`.

    python ahc008_v25_midpoint_live/make.py ahc008_v25_fast_static/solve.cpp ahc008_v25_midpoint_live/solve.cpp
    g++ -std=c++23 ahc008_v25_midpoint_live/solve.cpp -o ahc008_v25_midpoint_live/solve
    g++ -std=c++23 ahc008_v25_midpoint_live/unit.cpp -o ahc008_v25_midpoint_live/unit && ./ahc008_v25_midpoint_live/unit
    export PYTHONPATH="$PWD/ahc008_integrated_v11:$PWD/ahc008_focus_solver/problems/ahc008/experiments/20261122:$PWD/ahc008_focus_solver/problems/ahc008:$PWD/ahc008_bridge_audit"
    python ahc008_v25_midpoint_bench/bench.py ahc008_focus_solver/problems/ahc008 ahc008_focus_assets/tools/target/release/tester ahc008_v25_fast_static/solve ahc008_v25_midpoint_live/solve ahc008_v24_five_retention/m5-dev30/in ahc008_v25_midpoint_bench/reproduced.csv
    python ahc008_v25_midpoint_bench/id_compare.py ahc008_v25_midpoint_bench/reproduced.csv ahc008_v24_five_retention/m5-dev30/in
