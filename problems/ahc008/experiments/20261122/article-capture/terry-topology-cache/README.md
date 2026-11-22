# Exact faithful topology memoization

Derived pinned source 8fbfd770eeb2549346705fddb45172e31d3e0bde027ed2f891462073be5a083e
from selected motion-cache faithful d14f6471. Memoize three immutable
board-derived tables within a fixed semantic-map Normal controller:
- actual connected components keyed on actual wall mask;
- numbered room IDs, sizes and named doors keyed on actual and projected
  wall masks and the current majority group (recomputed for every caller);
- projected articulation flags by projected wall mask.

Component membership/admissible openings/storage are unchanged. The cache
handles alternate deployment of humans between components, including in
simulated policies. Not keyed on a case, seed, t or predicted outcome.
The unit compares all emitted topology fields and articulation bits on
1,500 independent synthetic snapshots, including different actor groups
on the same terrain/masks. GNU C++23 submission-form standalone builds.

Use the ordered motion-cache source locally first (it embeds private NN
data). This archive stores only the transform/fixture and independent
ledger, not source or binary with those weights. Rebuild with:

    python ahc008_terry_topology_cache/make.py ahc008_terry_motion_cache/submission.private.cpp ahc008_terry_topology_cache/submission.private.cpp
    g++ -std=c++23 ahc008_terry_topology_cache/submission.private.cpp -o ahc008_terry_topology_cache/solve
    g++ -std=c++23 -O2 ahc008_terry_topology_cache/unit.cpp -o /tmp/ahc008-topology-unit && /tmp/ahc008-topology-unit
    python ahc008_terry_speed/bench.py ahc008_focus_assets/tools/target/release/tester ahc008_focus_assets/tools/in ahc008_terry_exact_root/practice100.csv ahc008_terry_motion_cache/solve ahc008_terry_topology_cache/solve practice.speed.csv

## Measured fixed-work evidence

All 400 serial official invocations matched exact known legal independently
replayed 300-command SHA and scores on 200 case pairs. Use recorded late
pair counts in this *diagnostic only* to keep work identical in both
arms. Exclude tester calculations from wait4 solver CPU times.

| Paired panel, fixed work | Motion-cache CPU | With topology memo | Reduction |
|---|---:|---:|---:|
| Practice100 | 72.350090 s | 68.369807 s | 5.50142% |
| Independent100 | 85.668464 s | 80.671057 s | 5.83343% |
| Practice without rollout (86) | 40.020985 s | 37.515782 s | 6.25972% |
| Independent without rollout (81) | 41.163840 s | 39.054227 s | 5.12492% |

Maximum paired local tester wall 2.5372→2.4787 and 2.6454→2.4676 seconds.
Do not add reductions across experiments with different machines/controls,
and do not claim increased timed score. Default compatibility has its own
record; benchmark scoring high 6,082,666,664 practice is not a rank.
