# AHC008: keeping shared 1×4 cages useful with a small crew

All experiments preserve a shared-shoulder, two simultaneous gate model.
The selected comparator remains v7. Paired fixed development inputs (30),
official tester and official-output playback with independent score matching
passed for all arms; baseline hashes were compared against the original
frozen ledger. The initial-layout/passage experiments change only M=5/6
(10/30), with all twenty M≥7 outputs byte-identical to v7. No holdout or
historical placement inference for these rejected variants.

| Patch and parent | Cohort | Whole clear | Whole clear with >=300 nearby human cells | 4-cell captures | Remaining | Score |
|---|---|---:|---:|---:|---:|---:|
| wide_stage (v11) | 30 | 13→9 | 7→7 | 221→216 | 71→86 | 664,972,910→672,791,231 |
| low_cross_aisles (v7) | M5/6 | 2→2 | 1→1 | 76→58 | 30→46 | 160,449,518→86,149,192 |
| low_mid_aisle (v7) | M5/6 | 2→2 | 1→0 | 76→64 | 30→32 | 160,449,518→127,935,459 |
| low_dual_use (v7) | M5/6 | 2→1 | 1→0 | 76→68 | 30→24 | 160,449,518→126,412,109 |

On the larger map, retaining the absolute number of ready tubes for switching
out of one-pair mode starts full mode earlier (median 76 to 54.5 under paired
telemetry), and early true-four catches 92 to 108, but actually slows physical
ready-stock completion (t100 median 36.5 to 30.5). One global threshold cannot
protect construction and capture separately.

Three banks of three short runs (27 cages from 26) added cross-board service
aisles but needed 144 rather than 112 brace walls. The median first staffed
prepared bank worsened 49.5 to 54 turns. Maintaining the two-bank frame and
omitting both middle bodies left a floor passage with the same 112 brace
walls but forfeited two central traps. Letting both serve as empty open
crossings retained all 26 traps and reduced residual animals, but did not
produce another whole clear; actual first staff at 49.5 turns was unchanged
for both center variants. No variable is selected on case index.

Use [exact v7](https://github.com/tokotoko7777/ahc-workspace/blob/b7bb3d1bcae4c2a22dab85ad6d061841569370e8/problems/ahc008/experiments/20261122/article-capture/solve-v7.cpp)
or archived v11 as base. Reproduce a candidate with
`patch -o solve.cpp exact-parent.cpp < candidate.patch`, then
`g++ -std=c++23 solve.cpp -o solve` (optimization in source).
Compile the matching geometry fixture from this directory against the patched
`solve.cpp` when present. [Paired runner and ready-metric scripts](https://github.com/tokotoko7777/ahc-workspace/tree/843ebd510bd47a162b72caa901ec30a18a208416/problems/ahc008/experiments/20261122/article-wide-v11);
`*.csv.gz` contains exact action and input hashes, timings and outcome rows;
`*.json` gives group-wise summaries. Exact source checksums are saved separately.

Next trial uses the low-worker setup principle from [TERRY's article](https://blog.terry-u16.net/entry/ahc008):
compare wall-placement and egress-to-aisle completion as one operational
makespan, not nominal cage count or raw shortest travel. The original
baseline remains untouched.
