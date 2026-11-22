# Strict spare midpoint coverage in AHC008 (not adopted)

Follow-up to v25-shared-midpoint, after the separate 30-tape spare-only
feasibility screen. Starting from pinned v25-fast2, M5 regular phase only,
after constructor and stock commands. Keep all commands, busy holds,
still-open prior axial assignments and exact legal axial closures.
An extra pair may add only two otherwise free workers; verify both supported
tubes and every issued action jointly. Midpoint topology, weighted max over
the same two trap opportunities per trial, and original wall cells remain
as before. No geometry, midgame release or endgame change.

## Same M5 development 30

Official tester, exact official playback and independent scoring for both
arms. All 60 runs were legal.

| Metric | v25-fast2 | strict midpoint spare |
|---|---:|---:|
| Exact four by t100 | 68 | 68 |
| Exact four by t160 | 191 | 193 |
| Final accessible animals | 79 | 92 |
| All caught | 11/30 | 7/30 |
| All caught with mean human area ≥300 | 4/30 | 2/30 |
| Official total | 536,821,892 | 456,434,392 |
| Maximum local tester wall time | 0.574 s | 0.638 s |

There were 22 dispatch turns with 44 new first moves across 12 boards,
and **zero** submitted midpoint closures. Three score wins, five losses,
22 equal scores; 18 exact full action streams match. All three new
(at t160) stock-unmatched exact-four animals were already isolated by
stock before the end, in other shapes; the variant lost one stock exact-four
by t160. The 69 trace feasibility turns were not an expected success count.
The strict policy cannot promise unchanged later trajectories after an added
job, even though each current stock nonwait command is retained.

Reject; no held cases and no alteration to selected v25-fast2. Do not loosen
guard-displacement constraints to find another midpoint variant. Next explore
whether late conditional shared prep with different completion cuts produces
genuinely new high-area whole-capture opportunities.

## Reproduction

Extract the archive into workspace root. It includes the alternative
fragment and test, dependencies, both older midpoint shape/ranking fragments
used to generate, the matching script and exact dev ledgers. Source SHA
`00514609ae2122c38d0da1e7292511f6b2ad679e5abd16c8e13bc533e4d810e0`.

    python ahc008_v25_midpoint_spare_live/make.py ahc008_v25_fast_static/solve.cpp ahc008_v25_midpoint_spare_live/solve.cpp
    g++ -std=c++23 ahc008_v25_midpoint_spare_live/solve.cpp -o ahc008_v25_midpoint_spare_live/solve
    g++ -std=c++23 ahc008_v25_midpoint_spare_live/unit.cpp -o ahc008_v25_midpoint_spare_live/unit && ./ahc008_v25_midpoint_spare_live/unit
    export PYTHONPATH="$PWD/ahc008_integrated_v11:$PWD/ahc008_focus_solver/problems/ahc008/experiments/20261122:$PWD/ahc008_focus_solver/problems/ahc008:$PWD/ahc008_bridge_audit"
    python ahc008_v25_midpoint_spare_bench/bench.py ahc008_focus_solver/problems/ahc008 ahc008_focus_assets/tools/target/release/tester ahc008_v25_fast_static/solve ahc008_v25_midpoint_spare_live/solve ahc008_v24_five_retention/m5-dev30/in ahc008_v25_midpoint_spare_bench/reproduced.csv
    python ahc008_v25_midpoint_spare_bench/id_compare.py ahc008_v25_midpoint_spare_bench/reproduced.csv ahc008_v24_five_retention/m5-dev30/in
