# AHC034 cargo-feasible walk shortcuts

See [scoring and historical comparability](../../README.md): winner yosupo
9,486,350,384, published 150-case historical input set not established.
This experiment used a predeclared fresh 10 development + 16 untouched
seeds (`ahc034/chronological-shortcuts-v4/{split}/{i}` SHA-256 salt),
disjoint from prior trials and starter seeds. No final cases.

The [prior coverage-only shortcut](../physical_shortcuts_v3/README.md)
kept a visit to all nonzero cells but commonly removed a chronologically
necessary supply/deposit opportunity. In the new mode, a cut must retain
coverage **and** pass an exact chronological gate before adoption.
While scanning the virtual spliced route, adjusted visit counts identify
the first occurrence of positive-height cells and the last occurrence
of negative-height cells. Take all supply as early and satisfy demands as
late as possible. This one schedule maximizes available truck load at
every prefix, so a negative prefix rules out any legal complete flow.
Conversely, a nonnegative prefix is a legal allocating witness (there is
no finite truck capacity). The gate already has the visit deltas and
therefore does not materialize the trial walk; it spends at most one walk
scan per geometry proposal that passes coverage. Existing flow still
optimizes the actual weighted travel, independent of this witness.

Compare the same frozen annealed path and flow result, and preserve it
as legal fallback. All extensions have the common 1.85 s internal
deadline and exactly two extra flow solves per case. Modes are
0 = incumbent flow only, 1 = longer original search plus flow,
2 = coverage only plus flow, 3 = chronological gate plus flow. Modes 2
and 3 share span 24 / 56, both Manhattan L orientations and candidate
ordering. The gate in 3 changes only the cuts that can be committed.
Raw compute is compared with actual timings, not presumed equal.

| Split | Frozen flow / longer search | v3 coverage only | v4 chronology | v4 vs v3 W/T/L |
|---|---:|---:|---:|---:|
| Development 10 | 35,356,713.40 | 35,444,900.90 | 40,256,263.20 | 7/3/0 |
| Untouched 16 | 35,225,693.81 | 37,054,395.38 | 43,686,637.44 | 13/3/0 |

Untouched v4 vs frozen or longer-search: 15 wins / 1 tie, +24.0% ratio
of means (+25.8% mean paired ratio), median +7,640,349. Versus v3 the
median was +7,195,270.5. V4 accepted at least one profitable geometry
on 15 unseen cases, selected mean movement 652.25 vs frozen 1575.5,
and reduced total travel 201,719.38 → 160,201.88. Carried-load cost rose
44,169.38 → 94,976.88, but the road savings outweighed this; handling
was unchanged. V4 rejected 20,794 of 24,431 coverage-preserving proposals
on untouched seeds. It averaged 0.286783 s (max 0.391493 s) vs v3
0.273451 s and longer search 0.509683 s (max 0.751379 s).
All 104 outputs fully leveled the board and matched independent replay
and the official scorer; all work remained below the official limits.

Promote to the problem branch default only (no main merge or historical
claim). A single-file build with the default macro produces the exact
tested candidate binary. Randomized differential: streamed gate vs
materialized first/last event arrays, 2,511 feasible and 576 infeasible
coverage-preserving splices on 4,000 proposals. The 22 repository
regressions also passed. No generic API changes requested; use the
existing tested forward-time min-cost flow.

The next structural target, if reopened, is balancing carried and road
cost before shortlisting. Geometry-only selection now creates feasible
but too-loaded walks; use a low-cost first/last cargo proxy or
transport cuts to rank similarly short candidate geometries before exact
flow. No more experiments in this bounded slice.

## Reproduce

```sh
p=problems/ahc034
d="$p/experiments/chronological_shortcuts_v4"
python3 scripts/bundle.py "$p/main.cpp" -o /tmp/034-submission.cpp
for mode in 0 1 2 3; do
  g++ -std=c++23 -O3 -DAHC034_SHORTCUTS=$mode /tmp/034-submission.cpp -o /tmp/034-arm$mode
done
g++ -std=c++23 -O2 tests/ahc034_chronological_splice_test.cpp -o /tmp/034-chron-test
/tmp/034-chron-test
taskset -c 8 python3 "$d/benchmark.py" "$GEN" "$VIS" /tmp/034-arm0 /tmp/034-arm1 /tmp/034-arm2 /tmp/034-arm3 "$STARTER_SEEDS" "$WORK"
```

Original measured arms came from the frozen v3 and fixed-flow bundles:
baseline `0d4cf4d9c516b16cb140f226bc10447579a8c3e0802365c4e9218a7c10c5c4db`,
long `f0d62b1ede421a22e9efc2b813268ef64745bdd0f080f669830bfe2ca0c5759f`,
coverage `1d4bfe4e2113ff85b1204f0feb1a431c7c61d8ed8009bd445d4a62b7bec21c38`.
The mode-3 binary SHA is
`36eac6e6421130fdba806b88b8a1a38b2031d87e5c21e75d54aa92c553d2a07d`
(bundle with explicit mode: `101d6d0b41937596706dbfa5b1829ea7b249c305867ef514cef60ec4fe0ebff0`).
