# Pure-speed passive forecast reuse, same AHC008 article v4 policy

Reuse only C/P/R probability distributions keyed by the exact visible
wall board, species pace and current source cell. Extend each distribution
lazily as larger horizons are requested. Keep at most 256 current-board
entries; compute normally if full. Do not cache hidden or direction-dependent
dog or cat motion, permit any change in legality, alter candidate scores, or
replace a decision. Identical in-game actions are the promotion criterion.

| Sequential, alternating order paired official tester v3 | Distributed practice100 | Frozen independent generated100 |
|---|---:|---:|
| Score, both | 2,598,735,635 | 2,519,824,436 |
| All animals disconnected, both | 33 | 41 |
| Full action hashes equal to each other and frozen v4 | 100/100 | 100/100 |
| Median wall seconds, old→memo | .626823→.616169 | .632742→.605388 |
| Median of per-case relative changes | −3.605% | −2.810% |
| Cases faster | 82/100 | 75/100 |
| Maximum wall seconds, old→memo | .939411→.898666 | .966546→.872947 |
| Predicted-pet cache hits / lookups | 19,998 / 94,274 | 20,758 / 99,559 |
| Peak entries (capacity 256) | 256 | 208 |

Also independently legal and action/score identical on initial dev30,
paired median −6.155%, 23 faster. Unit comparison uses deliberately varied
wall epochs, all three species, increasing/decreasing horizons and full
pool fallback, checking every float byte against the original routine.
The two 100s additionally match per-animal capture event records exactly.
Tester wall time is not contest-server execution time. No score change
or AtCoder/main merge. Estimated memo storage ≤10 MB (not measured RSS).

```sh
D=problems/ahc008/experiments/20261122
A="$D/article-capture"
g++ -std=c++23 "$A/solve.cpp" -o /tmp/article-v4
g++ -std=c++23 "$A/solve-passive-memo.cpp" -o /tmp/article-speed
g++ -std=c++23 -Wno-return-type "$A/test_passive_memo.cpp" -o /tmp/article-speed-test
/tmp/article-speed-test
PYTHONPATH="$D" python3 "$A/bench_passive_memo.py" problems/ahc008 "$TESTER" /tmp/article-v4 /tmp/article-speed "$PRACTICE100" /tmp/article-speed.csv --n 100
```

Use the same fixed independent seed list and official generator as the
adjacent main research report. The compact archived timing CSVs retain raw
input/action checksums, exact score, capture totals, time and cache hits.
They omit duplicate detailed track telemetry; reproduce the complete
track logs with the benchmark. The candidate source's SHA256 is provided
by the research commit and test artifacts.

Source SHA256 `69183e12ec29fecdddc12ee8955f5a488000b8a906b4d51351f0e88abf64da5b`;
verified executable SHA256 `bd2ead06d7b42ab0f04a2b7fcdedcca6016e3d14ba5c4f589d8c061d10c8b013`.
