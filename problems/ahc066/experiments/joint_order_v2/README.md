# AHC066 joint delivery sequence and one-macro routing

The [frozen default](../../OFFICIAL_AUDIT.md) remains unchanged. Its completed
2,000-case official comparison was 1,352,700,378,901 vs kurakura's
1,893,330,847,366. This diagnostic used no historical inputs or case scores.
The official list is read only to assert disjointness. The preregistered
generated splits use SHA-256 salt `ahc066/joint-order-v2/{split}/{i}` for
16 development and 24 untouched cases; previous tracked seeds are excluded.

Both experiments retain the template answer and register a candidate macro
once at the original pose. For each macro, compute a pose-graph route
matrix for all headings of the resulting start, pickup and drop locations
using existing `UnitLexicographicBfs`; one cache is shared across order
evaluations. For any fixed pickup/drop order, four-state heading DP
minimizes keys, tie-breaking on expanded physical actions. Its best
path is reconstructed only when both printed and expanded caps can be
met; it is not a constrained Pareto DP and may discard feasible higher-key
routes with lower expansion. Strictly lower-key legal answers alone replace
the original. Every task is still carried directly from its own pickup to
its drop, keeping previously delivered baskets intact.

- Default: no added matrices.
- Fixed-order control (`-DAHC066_JOINT_ORDER=1`): six extra geometric
  dictionary choices, same original task order, four-heading DP.
- Joint (`-DAHC066_JOINT_ORDER=2`): six geometric dictionary choices,
  same DP but a pose-aware task-order seed and 47 seeded swap/relocate
  probes for each graph, or 288 order probes per case. Therefore the
  arms match expensive graph builds and wall limit (1.38 s internal,
  <2 s externally), not all scalar operations. Actual work/wall show
  how the cheap cached scoring changed the extra compute.

| Split | Frozen mean keys | Fixed-order mean keys | Joint mean keys | Joint vs fixed W/T/L |
|---|---:|---:|---:|---:|
| Development 16 | 234.8125 | 232.7500 | 232.8125 | 6/6/4 |
| Untouched 24 | 225.5417 | 224.3750 | 223.5000 | 10/9/5 |

Untouched joint vs frozen: 10 wins and 14 ties; median improvement zero
under either comparison. All 120 outputs delivered all balls with agreement
on raw score and replay. Peak wall: frozen 0.037579 s, fixed 0.088331 s,
joint 0.182033 s. Every added arm built all six matrices. The 0.875-key
untouched difference vs the control and essentially tied development do
not justify promoting this experiment or repeating the exact official test.
Keep the macros opt-in. To revisit structurally, try exactly two
re-registrations with a charged transition, splitting global deliveries into
macro-specific phases rather than multiplying near-duplicate words. Treat
expanded actions as a hard separate resource.

## Reproduce

Official tools: see [problem README](../../README.md). Use stock official
gen/vis, with the public seed list used only for disjointness.

```sh
p=problems/ahc066
d="$p/experiments/joint_order_v2"
python3 scripts/bundle.py "$p/main.cpp" -o /tmp/066-submission.cpp
g++ -std=c++23 -O3 /tmp/066-submission.cpp -o /tmp/066-base
g++ -std=c++23 -O3 -DAHC066_JOINT_ORDER=1 /tmp/066-submission.cpp -o /tmp/066-fixed
g++ -std=c++23 -O3 -DAHC066_JOINT_ORDER=2 /tmp/066-submission.cpp -o /tmp/066-joint
g++ -std=c++23 -O2 tests/ahc066_joint_order_test.cpp -o /tmp/066-test
/tmp/066-test
taskset -c 8 python3 "$d/benchmark.py" "$GEN" "$VIS" /tmp/066-base /tmp/066-fixed /tmp/066-joint "$OFFICIAL_SEEDS" "$WORK"
```

The control used the binary already frozen from `673d5843` and archived
with the 2,000-case result (SHA-256 `627b0e97deebd79f963107d9e91f68a456e5b2461d7bf5a928660bf2a96ca4e7`).
Compiled bundle SHA-256 `8b9544c2c847d7befd0ba75fa1229711c1a167ec504cc2688720395f2977e0cb`,
fixed binary `55f558bceaad1f33f8f3239ef0949f7b589fe143662f61048865e2e21d296452`,
joint binary `19bf61a428e95e6ce8a898cd18e3373a212a5ecd4a2d9598b612e2786c4b444a`.
The differential test compares both orders across five distinct macros
against exhaustive `4^4` terminal-heading assignments and checks
route reconstruction's key/expanded lengths. Existing BFS was reused
without changes; no new generic library component was needed.
