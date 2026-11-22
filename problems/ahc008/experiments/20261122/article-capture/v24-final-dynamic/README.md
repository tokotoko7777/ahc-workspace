# AHC008: simulate staffing and partial walls for the final plan

This non-adopted ablation starts from exact v24. Only the five-worker
last-1-or-2 fallback is changed. Compare the three distinct enclosed
shapes with highest static score. In paired branches, execute the original
remaining-wall planner on each chosen enclosure (including temporary walls
and actual worker movement), up to 16 simulated turns. Stop after its
cancellation rather than re-search inside the rollout. At most six paired
trajectories per comparison (at least four to make a decision); accept an
alternative if mean gain exceeds one paired standard error. About 60 ms
total process CPU budget for the extra comparisons, 14 ms per decision.

Same M5 development 30 as the
[parent checkpoint](../v24-final-planner/README.md). All 60 official and
independently replayed; 434,537,087 → 433,453,753, whole 10 → 10,
whole with ≥300 2 → 2, final animals remaining 62 → 62. Three action
histories changed. Replaying those three exact histories produced
(one, two, one) simulated policy changes and 42.1ms, 60.0ms and 7.7ms
extra simulation CPU. Maximum tester wall clock 0.675s. No adoption and
no tuning or test on the previous independent panel.

The bottleneck here is not simply absence of forecasts: the same flow
search generated these similar shapes, and simulation chose a different
option only four times in the three affected cases. A separate next
hypothesis worth testing is to generate alternatives that are genuinely
different in where and how the last gates cut the service graph, then
select those with shorter and safer closure sequences.

Run `python make.py /path/to/solve-v24.cpp solve.cpp`, check the hash,
and compile `g++ -std=gnu++23 solve.cpp -o solve`. Exact benchmark
commands and development seed manifest are in the parent checkpoint.
