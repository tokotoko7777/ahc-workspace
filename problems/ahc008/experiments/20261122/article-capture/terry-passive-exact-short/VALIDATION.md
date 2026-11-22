# AHC008: exact short passive prediction

Accepted as the new research reference, not merged into main. Under the existing
mixed-room policy, this change replaces the passive cow/pig/rabbit room-retention
mass at ETA 1–3 with the exact value on the current tentative adjacency. It
preserves the original sampled counts and complete RNG stream, directed pet
sampling, room geometry, actual closing logic and stop conditions.

| Inputs (100 each) | Prior topology cache | Exact short | Change | Full before → after | Max tester wall after |
|---|---:|---:|---:|---:|---:|
| Fresh development | 6,082,333,335 | 6,116,055,555 | +0.55443% | 98 → 99 | 2.74722 s |
| Fresh precommitted holdout | 6,019,777,776 | 6,054,388,889 | +0.57496% | 98 → 99 | 2.81200 s |
| Official distributed practice | 6,082,666,664 | **6,093,833,333** | +0.18358% | 98 → 98 | 2.74622 s |

All 600 solver runs passed the official tester, official action playback,
and independent replay/scoring. The preregistered gate (all legal; local
tester max <2.95s; no drop in full and useful full count; >=0.1% score gain)
passed on all three sets, in that order. The practice set has 12 wins, 77
ties, 11 losses; 48 action traces differ. One case (0093) gains a full
capture, one (0061) loses it, and 0084 still leaves one animal. Two animals
total remain in each arm. Outside the two flips the net gain is 5,777,779.
The official distributed practice gap to the historical reference score of
6,263,333,336 is 169,500,003 numerically; the test cases differ and this
does not establish a ranking.

Candidate C++23 source SHA-256:
`08bff3c8aaeb6613b47be82b44b12bf90f63bd80476b69498bb7bc9e63fa375c`.
Control C++23 source SHA-256:
`8fbfd770eeb2549346705fddb45172e31d3e0bde027ed2f891462073be5a083e`.
See README and PILOT_GATE for unit verification, audit and build. For
reproduction use the topology-cache source and the separately supplied
selector data as outlined in the preceding archive; no third-party source,
selector data or executable is bundled here. From the workspace with the
official tester and prior experiment layouts, rerun the paired harness:

```sh
export PYTHONPATH="$PWD/ahc008_integrated_v11:$PWD/ahc008_focus_solver/problems/ahc008/experiments/20261122:$PWD/ahc008_focus_solver/problems/ahc008:$PWD/ahc008_bridge_audit"
python ahc008_terry_passive_exact_short/make_live.py
g++ -std=gnu++23 ahc008_terry_passive_exact_short/live.private.cpp -o ahc008_terry_passive_exact_short/live
python ahc008_terry_exact_root/bench.py ahc008_focus_solver/problems/ahc008 ahc008_focus_assets/tools/target/release/tester ahc008_terry_topology_cache/solve ahc008_terry_passive_exact_short/live CASE_DIRECTORY result.csv --n 100
```

Use the practice data linked by the [official task](https://atcoder.jp/contests/ahc008/tasks/ahc008_a)
or generate the development and holdout inputs using the respective seed
lists and the official generator, then verify their SHA256SUMS.
Per-case timing and branches that use time remaining can vary by system.
The bundle contains source transforms, pure algorithmic tests, the read-only
tape audit, each full result CSV, seed lists and input hash manifests.
