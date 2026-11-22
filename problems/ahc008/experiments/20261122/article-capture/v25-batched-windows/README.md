# v25 batched capture-window reductions

Build from exact v25:
    python ahc008_v25_batched_windows/make.py ahc008_v24_standby_early_seam/record/solve-v25.cpp ahc008_v25_batched_windows/solve.cpp
    cd ahc008_v25_batched_windows
    g++ -std=c++23 solve.cpp -o solve
    g++ -std=c++23 unit.cpp -o unit && ./unit

Only guard-window readout changes. On the first call, cache the fixed selected geometry's per-cell body and blocking tube masks. Each requested ETA shares one pass across trial pets to OR the blocked tubes and the occupied tubes, then sums valid per-tube captured heads and counts trials with at least one. Each pet is counted even when several occupy the same cell; each gate-contacting pet disqualifies the entire tube for that trial. Masks accommodate shared body/shoulder geometry and count each tube only once for each head. Numerators are exact bounded integers, divided once by the same trial count as prior samples. Production needs only expected and any; optional per-pet readout is used for full exact unit comparison. The work generates and interprets the same sampled paths and pairInfo; callback writes the same per-tube/horizon cells, with downstream tie enumeration untouched. The original standalone reducer remains untouched and is invoked if there are more than 64 tubes.

The unit covers all triples on eight chosen cells and all subset masks/horizons (including invalids) for overlapping gates/bodies, duplicate cells and shared blockers; the 64th-bit case, >64 initializer refusal, and 900 deterministic random paths up to 128 trials and twenty pets, both production aggregate and per-pet lanes against the original bitwise double results. No official cases were run here. Absolute budget-limited behavior can still change with speed; require exact action replay and compare uninstrumented elapsed sequentially before declaring a speed win.

## Development speed panel

Read [the exact v25 profile](profile/README.md) for the measured baseline
hotspots. Uninstrumented v25 vs batched variant, same 30 five-worker
cases, three measurements per case with alternating arm order and no
co-running official jobs. All 90 paired official action hashes and
scores identical. wait4 solver-only process CPU 42.3393 s → 39.1506 s
(−7.53% aggregate; paired median −7.91%). End-to-end local tester wall
time 43.0480 s → 39.8850 s (−7.35%, including minimal wrapper overhead).
Both 100-case confirmations are separate.

## Broader timing validation and speed-only adoption

| Exact input set | Solver CPU before → after | Reduction | All commands identical | Before turn 275 |
|---|---:|---:|---:|---:|
| Official distributed 100 | 59.720101 → 51.489724 s | 13.78% | 99/100 | 100/100 |
| Independent general 100 | 61.863878 → 52.912421 s | 14.47% | 100/100 | 100/100 |

All 400 runs were valid with the official tester and independent
replay. Scores practice 3,294,019,394 → 3,293,908,283;
independent 3,358,241,534 unchanged. The sole late mismatch is the
original clock-budget-dependent rollout on practice0018; repeating
old v25 produces both outputs under different paired simulation
counts. See `clock-repeat.txt`. This is not an improvement claim in
points or rank. v25-fast has unchanged deterministic search counts/
policy, and only takes less CPU to reduce the same guard samples.
Maximum local wall time 0.960 s (practice) / 0.926 s (independent).
No AtCoder submission or main merge.

Verified standalone: `g++ -std=gnu++23 solve-v25-fast.cpp -o solve`.
The logs and source generators are in `evidence.tar.gz`. The separate
[small batch-window API proposal](reuse-proposal.md) is not applied
to the common library yet. No general speed extrapolation.
