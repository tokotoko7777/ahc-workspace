# AHC004: source-only hot path on GCC

With GCC 14.2 a source `#pragma GCC optimize("O3")` is already active,
but using the driver without `-O3` still left many out-of-line libstdc++
accessor calls in dispatched hot functions. The POPCNT clone itself was
present in both builds. In the original no-driver-flags disassembly,
the cached 800-placement loop calls both vector and array indexing.
The original Aho scan similarly calls vector/string indexing plus
`AhoCorasick::advance`, `matches` and iterator helpers for each character.
The driver-`-O3` build optimizes/inlines these calls; enabling POPCNT on
more machines does not address this compiler difference.

`AHC004_SOURCE_HOTPATH` preserves the transition algorithm while
hoisting rotated cache storage pointers, keeping the rerank arrays plain,
and using the same full-width unbiased integer reduction as GCC 14's
libstdc++ implementation on supported targets (the standard routine is
retained elsewhere). A packed state mirror and fixed scratch board avoid
copying 20 std::strings on each trial. A flat immutable Aho node/output
cache removes wrappers from the per-character scan while preserving
output IDs, duplicates, cyclic crossing, and blank handling. Apply only
accepted character changes to the ordinary board and keep the same
coverage/state interface. Runtime-selected hardware POPCNT is unchanged.

Pinned CPU4, fixed 25,000 synthetic proposals **and exact delta
evaluations**, alternating executions, three repetitions:

| Arm | Median milliseconds |
|---|---:|
| Old, driver defaults | 440.106 |
| Source hotpath, driver defaults | 155.406 |
| Old, explicit driver `-O3` | 131.071 |

The proposal/delta fingerprint (5082622593642394555) and following RNG
word (17061058872283092397) matched all nine runs. The source-only
variant was 2.83× the former source-only throughput, reaching 84% of the
legacy explicit-`-O3` reference, in this microbenchmark only. No official
quality score, contest wall or rank was evaluated here.

## Build and verify

```sh
d=problems/ahc004/experiments/source_hotpath
python scripts/bundle.py "$d/submission_entry.cpp" -o submission.cpp
g++ -std=c++23 submission.cpp -o solution
g++ -std=c++23 tests/ahc004_source_hotpath_test.cpp -o /tmp/hot-test
/tmp/hot-test

g++ -std=c++23 -DAHC004_FAST_PLACEMENT -DAHC004_EXACT_PLACEMENT "$d/micro.cpp" -o /tmp/old
g++ -std=c++23 -DAHC004_FAST_PLACEMENT -DAHC004_EXACT_PLACEMENT -DAHC004_SOURCE_HOTPATH "$d/micro.cpp" -o /tmp/new
g++ -std=c++23 -O3 -DAHC004_FAST_PLACEMENT -DAHC004_EXACT_PLACEMENT "$d/micro.cpp" -o /tmp/opt
for i in 1 2 3; do for arm in old new opt; do taskset -c 4 "/tmp/$arm"; done; done
```

The source entry selects fast/rerank/hotpath at preprocessing time with
no extra driver options. For a different search combination, add ordinary
project `#define` lines before the main implementation or build it
directly with those macros enabled. No parent branch or default was
changed. The differential test targets densely matching cyclic boards,
short and long patterns, duplicate hits, blank edits, rotations and swaps;
each accepted incremental state is cross-checked against a full rebuild.
