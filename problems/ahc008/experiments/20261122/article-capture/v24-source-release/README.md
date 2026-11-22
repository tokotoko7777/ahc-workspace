# AHC008 v24: construction-first and live lane release

Research baseline v24 keeps the earlier v7 two/three-bank exact 1×4
geometry and conditional early/late fallback, with the TERRY-module front
end. Two independently written changes start construction before capture
allocation, retain temporarily blocked FIFO work, and recompute all-worker
bottleneck lane slots at the nearest reachable point. Current legal paired
closures still take priority over speculative captures for workers not
reserved by construction. The authorized Rust supplied for inspection is
not published here and was not executed.

| Same frozen inputs, v7 → v24 | Official distributed100 | Independent generated100 |
|---|---:|---:|
| Score | 2,655,369,894 → 3,199,806,134 | 2,688,102,365 → 3,285,615,612 |
| All animal isolation | 36 → 47 | 44 → 44 |
| All isolated with mean human floor ≥300 | 25 → 33 | 26 → 35 |
| Exact four-cell captures by t100 / final | 433→505 / 822→921 | 456→506 / 837→954 |
| Remaining accessible animals | 165 → 117 | 139 → 148 |

Development30 from v7: 829,740,689→981,530,056, full14→15, useful9→12.
The development ledger's baseline arm is the same-geometry hybrid without
construction/release changes; v7 numbers in metrics.json correspond to
its separate previously verified history for these same input hashes.
Held results include all 400 verified legal official + independent
replays, with all 200 v7 scores/input and action hashes unchanged.
Worst observed candidate tester wall1.074 s, below the 3 s limit.
The independent raw all-head clear count is tied and ultimate residual
worsened in that cohort, but broader useful clears and score both improve.
Adopt as the research comparator, not a historical winner or main merge.

Reproduce `g++ -std=c++23 solve-v24.cpp -o solve-v24`, then pass
v7 and this binary to `../terry-integration/bench.py` with the relevant
official tester and the unchanged practice or independent100 input folder.
Paired late Monte-Carlo includes a CPU reserve; on an overloaded system
the candidate's completion count can change a late decision.
SHA256 of single source:
`7ae8fc4c25a5b69c448682b62c428fb73f83b7a2d826003f07b2ea173be06639`.
