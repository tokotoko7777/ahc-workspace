# AHC008: independent C++23 implementation of the specified TERRY method

Reference: TERRY's [article](https://blog.terry-u16.net/entry/ahc008) and
the user-supplied text of [submission 29650242](https://atcoder.jp/contests/ahc008/submissions/29650242).
This is a separately written C++ solver and an audit, not a build or execution
of the original Rust. The original text, weights and assembled file/binary
carrying private user-supplied weight data are not in this archive.

## What changed relative to our earlier adaptation

1. Keep five fixed layouts with the original one-mouth and two-mouth
   variable-sized rooms. No transposition, no forced 1×4 conversion. Static
   checks reconstruct the entire layout marker fingerprints. Fully constructed
   room counts (one/two mouths) are map0 46/7, map1 62/14, map2 30/45, map3 15/60,
   map4 54/21; actual dynamic rooms depend on the fences and actors.
2. Same normalized counts and neural net choose among the five. The verified
   independent selector agreed exactly on each output for 1,780 inputs with
   an independent decoder and scalar evaluator. No instance/seed IDs are used.
3. Use both original snakes including both zero-based columns 0/29 and the
   last interior row, two independently reversible ends per assigned interval,
   strict FIFO with intentional waits, then a repeated bottleneck assignment
   to physical aisle columns; no artificial release turn.
4. Recompute reachability and legal rooms on observed/projected terrain as
   specified. Per pet already inside a numbered room, run 100 trials of 30
   forecast states, count occupancy of the same room including re-entry, and
   use no capture credit for incoming unrelated animals. Pairs/singles use
   the source's worker and room exclusivity and deliberate legacy prune.
   After those commands, remaining staff follow unique animal row
   assignments within each aisle, with separate assignments across aisles.
5. At turn 275, compare complete ordinary and chase policies on paired seeds.
   Chase, if selected, exclusively uses a newly calculated finite-prize vertex
   cut each turn. In the reference rollout the normal branch reuses mutable
   construction queues, and the simulated chase branch passes turn 275 to
   the action controller on all 25 simulated turns; live chase uses live turns.

C++ control is intentionally not claimed byte-identical to the original:
no native run or original command trace; score/behavior was tested against
our own frozen baseline, not against TERRY's system case results. A strict
safety wall-clock cap (2,670ms since start, only completed pairs counted)
replaces the original external loop-start check at 2,700ms. In unanticipated
isolated synthetic states we use legal no-op/fallback instead of panicking.
The output sanitizer rejects duplicate walls or invalid moves independently.
Any remaining observed flow-tie differences are documented below.
Internal step RNG follows the public [rand 0.7.3 uniform](https://raw.githubusercontent.com/rust-random/rand/0.7.3/src/distributions/uniform.rs),
[small slice indices](https://raw.githubusercontent.com/rust-random/rand/0.7.3/src/seq/mod.rs),
[PCG64 MCG](https://raw.githubusercontent.com/rust-random/rand/0.7.3/rand_pcg/src/pcg128.rs),
and [rand_core u64 seeding](https://docs.rs/rand_core/0.5.1/src/rand_core/lib.rs.html)
mechanisms. Source algorithms from that licensed upstream were used as
technical references; no raw participant code used in the implementation.

## Reproduction

Work in the workspace root. Extract the archive, and provide a private
local copy of the originally supplied authorized text. The parser only
reads its model data and does not run it. The generated standalone and
its coefficients must stay local.

    python ahc008_terry_exact_selector/extract.py /local/path/to/authorized-reference.txt
    python ahc008_terry_exact_root/flatten.py ahc008_terry_exact_root/submission.private.cpp
    g++ -std=c++23 ahc008_terry_exact_root/submission.private.cpp -o ahc008_terry_exact_root/standalone

The bench uses the original official v3 interactive tester, independent
playback and independent score. See the frozen control's experiment archive
(v25-fast2, d6eafd3ff9edb8f55421e1df903113d30efbd1ce). On same exact panel
positions always compare pairwise and do not run multiple testers at once.

    export PYTHONPATH="$PWD/ahc008_integrated_v11:$PWD/ahc008_focus_solver/problems/ahc008/experiments/20261122:$PWD/ahc008_focus_solver/problems/ahc008:$PWD/ahc008_bridge_audit"
    python ahc008_terry_exact_root/bench.py ahc008_focus_solver/problems/ahc008 ahc008_focus_assets/tools/target/release/tester ahc008_v25_fast_static/solve ahc008_terry_exact_root/standalone ahc008_focus_assets/tools/in practice100.csv --n 100

Before using a benchmark for adoption, require official legal for every
case, independent score agreement, and time reserve. A passed distributed
100 does not certify the distinct original system judging or allow a
conclusion about historical first place.

## Frozen official distributed 100

GNU C++23 standalone source hash (private generated file):
`54f2b1ecc90820fcc5c99bc4e3cb8ddfe06c5952bf45851bf9d1a21e49341ae9`.
All 200 runs (paired two programs on each board) had official legality,
exact official playback and independent scoring. Official practice inputs
are not the original system set. Compare this exact full solver on the same
100 to the pinned current v25-fast2. It is not a direct Rust reproduction
benchmark.

| Metric | v25-fast2 | independent TERRY method |
|---|---:|---:|
| Score | 3,293,908,283 | 6,082,666,664 |
| All animals isolated from every human | 47/100 | 98/100 |
| Same and mean human area ≥300 | 34/100 | 98/100 |
| Total final residual | 123 | 2 |
| Mean final reachable area per human | 455.39 | 553.00 |
| Maximum local tester wall time | 0.855 s | 2.754 s |

The remaining two boards have one animal each. Corrected early 30 five-person
boards with the generic min-cost tie backend (intermediate, not this frozen
binary): 536,821,892→1,641,722,220, all isolated 11→28, mean area ≥300 among
all-isolated 4→28. The earlier border-incomplete C++ diagnostic is archived
in the CSV, labeled appropriately; it is not the final score. Root switched
both idle and chase assignment to a scoped min-cost flow backend with the
reference equal-distance/larger-vertex, early-sink, strict relaxation behavior
and verified it with two exact fixtures and 1,200 brute force small objectives.
It does not change the generic library.

The target 6,263,333,336 is 180,666,672 points above this run. As the old
system/ranking cases differ, this is a numerical practice target comparison,
not a statement about placement. No main change or online submission.

## Independent generated 100: confirmed

The same frozen conditional-TERRY binary (no patch, tuning or map overrides)
was tested serially against v25-fast2 on the pre-existing independent 100
from `ahc008-article-bench/full-capture-independent100/in`. Both arms legal,
exact official playback, and independent-score agreement on all 200 runs.

| Metric | v25-fast2 | independent TERRY method |
|---|---:|---:|
| Score | 3,358,241,534 | 6,037,488,888 |
| Whole isolation | 45/100 | 96/100 |
| Whole isolation and mean area ≥300 | 36/100 | 96/100 |
| Total final residual animals | 126 | 5 |
| Maximum local tester wall time | 0.859 s | 2.762 s |

We adopt this as the research benchmark (both case groups are much better),
not as an original contest rank and not on main. Time-budgeted switch
evaluations can run a different count of terminal pairs on a loaded machine.
A new improvement should preserve this fixed reference and verify legality,
area, full captures, speed and seed separation. Even clearing both last
practice animals with no area lost would add only about 61.78 million
points. The next improvement needs substantial retained area as well;
a fixed-final-graph analysis can bound the opportunity but cannot justify
illegally removing walls or show that changing earlier actions works.
