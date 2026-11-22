# Reusing the empty max-flow arena shortens Chase computations

The accepted faster split planner spends a substantial part of the
late simulations in the named Chase cut sequence. This experiment
reuses graph allocations across cuts using the existing library
clear_edges(). Each solve inserts exactly the original arcs, in
the same original order, with no inherited residual capacity.
A storage arena may have extra isolated vertices after a larger
previous target count. It does not alter original vertex IDs,
reachable masks, flow, mincut or shape. New occupancies/walls are
unconditionally encoded afresh. The scratch is thread-local and
used only on a single, nonnested call at a time. Both C++23 build
and 400 synthetic resetting/growing network comparisons passed.

| Fixed work | Cases | Accepted split wait4 | Flow-reuse wait4 | Saved |
|---|---:|---:|---:|---:|
| Distributed practice | 100 | 48.497672 s | 45.837923 s | 5.4843% |
| Independent generated | 100 | 53.446282 s | 49.709467 s | 6.9917% |
| Practice, positive late rollouts | 12 | 27.755122 s | 25.197258 s | 9.2158% |
| Independent, positive late rollouts | 14 | 33.517322 s | 30.037006 s | 10.3836% |
| Practice, zero late rollouts | 88 | 20.742550 s | 20.640665 s | 0.4912% |
| Independent, zero late rollouts | 86 | 19.928960 s | 19.672461 s | 1.2871% |

All 400 official fixed invocations reproduce independently verified
frozen entire 300-command and score digests using common capped
workloads. Use direct measurements; do not add these percentages
to the older speed gains. Max local tester wall 2.6401→2.3332
and 2.7343→2.5288 s. The optimization is valid at fixed work;
it need not improve actual decisions on a particular default
time-limited trajectory. See the companion normal-time validation.

C++23 candidate source SHA-256:
`fe77bd4c23180343ad4239bbd8f068d3cdb718eb5c8460b14994a798cf7c3625`.
Reproduce using pinned accepted split-DP source, make.py, build
unit.cpp and solve.private.cpp, and the same exact-cap paired harness.
Only independent tooling/tests/logs are archived, not supplied
selector weights, generated standalone with them, or executable.
