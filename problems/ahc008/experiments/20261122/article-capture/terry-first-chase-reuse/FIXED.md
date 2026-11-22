# Exact first simulated Chase action reuse

Each paired Chase arm starts from the exact same observed state and
fixed planner turn, and the Chase proposal is deterministic and pure
within that call. Evaluate its first action once lazily per decision;
all future simulated actions, actual actions, pet RNG, original
Normal mutation and score comparison are unchanged. 26 synthetic
multi-pair fixtures reproduce decision aggregates, queues and live
RNG with fixed work. Original default time can now finish more pairs,
which is a separate actual-score question.

| Frozen fixed-work comparator | Cases | Accepted wait4 CPU | Reuse wait4 CPU | Saved |
|---|---:|---:|---:|---:|
| Official distributed | 100 | 57.492319 s | 55.372886 s | 3.6865% |
| Independent generated | 100 | 65.936282 s | 63.139362 s | 4.2419% |
| Distributed positive-rollout only | 14 | 36.330758 s | 34.462943 s | 5.1411% |
| Independent positive-rollout only | 18 | 46.616277 s | 43.902375 s | 5.8218% |

All 400 serial official runs exactly reproduce known complete command
streams and scores. Maximum tester wall2.72121→2.65276s and
2.71709→2.58252s. Both preregistered positive-stratum and aggregate
CPU gates pass. The default admission test is separate. Exact source
SHA256 8f50dda74f578fc8ddbf8176d15172a80df96d23d82c57db8ae51aedce4294b7.
Pinned source transform and evidence only, no model weights, generated
solver carrying them, or executable.
