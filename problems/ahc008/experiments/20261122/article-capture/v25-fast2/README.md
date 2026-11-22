# AHC008 v25-fast2: exact static sampler reuse

v25-fast already batches the guard-window reduction; this separate change
reuses one trial state container and caches each visited cell's static
open neighbors in the same order. Trace RNG indices, distributions, dog
target updates, planner/capture rules, trial count and all dynamic
simulator calls remain unchanged. The original paths and internal jumps
matched in 3,500 deterministic randomized unit worlds before timing.

All timing repetitions used same-case alternating uninstrumented
official solver runs. wait4 reports observed solver-only CPU, not judge
time. Separate independent replay was performed on all 400 broad runs.

| Panel | Previous → new solver CPU total | Paired reduction | Action digest equality |
|---|---:|---:|---:|
| Static-only dev30×3 vs original v25 | 43.429987 → 41.982140 s | 3.33% | 90/90 |
| Combined fast+static dev30×2 vs fast | 27.718454 → 26.770349 s | 3.42% | 60/60 |
| Official distributed100, vs fast | 51.135465 → 49.391251 s | 3.41% | 100/100 |
| Separate general100, vs fast | 52.664287 → 50.909803 s | 3.33% | 100/100 |

Both broad scores identical (3,293,908,283 and 3,358,241,534), all
commands match; no score or ranking gain is claimed. Max local wall time
0.870s (practice)/0.865s (independent). These are additional paired
reductions relative to v25-fast; don't add separate benchmark
percentages or claim exact old→new elapsed without a direct run.

Selected speed-only baseline v25-fast2, no main merge. Compile the
frozen standalone with `g++ -std=gnu++23 solve-v25-fast2.cpp -o solve`.
Source SHA-256 `333f5177eebb5ca6f4a7cac4c0da5aae72e7aa82266803f97abfc06580b82345`.
See the [batched previous version](../v25-batched-windows/README.md)
for exact prior source, unchanged input sets and overlap. Raw independent
records, generators, unit and timing harness are in `evidence.tar.gz`.
