# Compact board-wide three-bank M5 geometry, with segment-local construction

The fixed candidate moves from two banks of thirteen to three banks of nine
with total 132 shoulders, exactly two-ended 1×4 traps, unchanged body/gate
length, four connected aisles and no change outside M5. See geometry
certificate and unit. Splits skip four consecutive original odd slots starting
at (2,6,2); every longitudinal row remains represented. Old parity sweep is
local to each contiguous run rather than repeatedly crossing the omitted slots;
the exact allocator, mirrors, blockers and catch/rescue precedence are stock.
The dry median construction estimate increased 63→70 across 18 paired
fixtures, not an estimate of actual staffed capture time.

## Official development result: rejected

All 60 serial tester runs and independent playback/replays legal on the same
historical M5 30. No held run and no alternative geometry entered official test.

| Metric | v25-fast2 | compact 3×9 |
|---|---:|---:|
| Brace-ready traps across 30 at t75 | 739 | 706 |
| Brace-ready traps at t100 | 751 | 797 |
| Median first staffed and executed trap lock, across banks that locked | 91 | 102 |
| Exact four by t100 | 68 | 40 |
| Exact four by t160 | 191 | 148 |
| Final accessible animals | 79 | 133 |
| All caught | 11/30 | 7/30 |
| All caught and mean human area ≥300 | 4/30 | 1/30 |
| Official total | 536,821,892 | 235,769,947 |
| Max local tester wall time | 0.586 s | 0.629 s |

Seven score wins, 23 losses. All five species worsened in final residue
(cow +14, pig +9, rabbit +6, dog +7, cat +18). The new geometry does close more
total banks (65 bank-level first locks versus 58, out of 90 versus 60 possible);
count and median are not the same metric. Better reach and more physical traps
at t100 were insufficient; stock assignment actually began capturing later
on this geometry. No causal decomposition of geometry vs paths/staffing
and no conclusion about other three-bank maps. Do not try more gap positions
against these already observed cases. Selected v25-fast2 remains unchanged.

## Reproduce

Extract evidence in workspace root, use frozen fast2 source
(d6eafd3ff9edb8f55421e1df903113d30efbd1ce). Dry and fixture commands are
in the included candidate README and topology README. Candidate source
SHA-256 `7cd1da669b499c7952898671eeac5ce5f47ec0f2cdd9507106db2e598f8e22e8`.

    python ahc008_v25_3bank_segmented/make.py ahc008_v25_fast_static/solve.cpp ahc008_v25_3bank_segmented/solve.cpp
    g++ -std=c++23 ahc008_v25_3bank_segmented/solve.cpp -o ahc008_v25_3bank_segmented/solve
    export PYTHONPATH="$PWD/ahc008_integrated_v11:$PWD/ahc008_focus_solver/problems/ahc008/experiments/20261122:$PWD/ahc008_focus_solver/problems/ahc008:$PWD/ahc008_bridge_audit"
    python ahc008_v24_early_exact/bench.py ahc008_focus_solver/problems/ahc008 ahc008_focus_assets/tools/target/release/tester ahc008_v25_fast_static/solve ahc008_v25_3bank_segmented/solve ahc008_v24_five_retention/m5-dev30/in ahc008_v25_3bank_compact_bench/reproduced.csv
    python ahc008_v25_3bank_compact_bench/summarize.py ahc008_v25_3bank_compact_bench/reproduced.csv
