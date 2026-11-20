# AHC072 — Somersault Slimes

[Canonical official statement](https://atcoder.jp/contests/ahc072/tasks/ahc072_a) ·
[Official tool pack](https://img.atcoder.jp/ahc072/BrsBAXF2.zip) ·
[Official ranked results](https://atcoder.jp/contests/ahc072/results/json).

Raw score is `operations + 100000 * remaining_slimes`, smaller better.
The relative final sum is the sum of the 2,000 individually rounded
`10^9 * minimum_valid_raw / own_raw` values. Reading the [official score
matrix](https://img.atcoder.jp/ahc_standings/ahc072/result.csv) and
reconstructing positive-case minima and integer rounding across the
1,056 users gives **mech_39, 1,995,670,086,683**, matching ranked results.
Score matrix SHA-256
`a67a32d6e01197c53bee4b616c21c77f978c03005b1071ec03bf10bff1705a6d`.
[Official case metadata](https://img.atcoder.jp/ahc_standings/ahc072/input.csv)
contains file / seed / N / K / M / walls, SHA-256
`ddba4faf734b71fe1b58bc35696fdeb51af863ff7bf0938406f972d63bc25770`.
The [published seed list](https://img.atcoder.jp/ahc072/seeds.txt) is
expected to hash to
`67cf8463e118af25951c5e583a5b41607e58143174ac6aa6ca8477285fa70323`.
This experiment does not use final cases: **unjudged**. Archived winning
raw mean 194.632 is not the same sample as the generated benchmark below.

## Transient support hops, then same-color caravans

Both arms start from the farthest unfinished slime by static wall
distance to its own nest. The control sends one slime home at a time,
using an exact state-dependent shortest route. When the traveler sits
above a stationary slime, it can jump length 2 by leaving that foundation
behind; otherwise only length 1. The path checks every crossed cell.
Static graph BFS could not express these changing arc lengths, so
precomputed wall rays and exact mutable support tests stay problem-local.

The variant optionally picks up another same-color stationary slime along
the way. The active group remains all of one color, so flips do not change
its effective order. At most seven active members are permitted to land
above one stationary member without exceeding height eight. All cells
outside this one moving group retain at most their original single
stationary slime. Traversing a nest of a different color is harmless;
entering the actual destination terminates the route, as all moving
members return home. Recompute pickup/terminal routes after each merge.
Retain the legal single-slime route if it is faster.

| Fresh cases | Supported singles ↓ | Caravans ↓ | Better / tie / worse |
|---|---:|---:|---:|
| Dev (30) | 855.900 | 433.000 | 30 / 0 / 0 |
| Holdout (30) | 639.933 | 345.667 | 30 / 0 / 0 |

Holdout median saving 219.5 operations. All 120 plans send all slimes home
and match the official score and independent replay, including both
endpoint pops, vector reversal, every wall, cap before return and all
jump lengths. Max observed wall .055364 s. Caravans are branch default.
[Logs](benchmark-results.csv).

**Next hypothesis:** temporarily combine *two color blocks* when both
share a long transport corridor. Model their order by jump parity; at
the first nest, the expected color must be on top before the automatic
pop, then deliver the surviving block to its own nest. Compare exact
legal route cost and subsequent fallback against independent caravans on
the same board, preferably with same candidate work and fresh cases.

```sh
p=problems/ahc072
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC072_CARAVAN=0 "$p/main.cpp" -o "$d/base"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/caravan"
python "$p/benchmark.py" /path/to/gen /path/to/vis \
  "$d/base" "$d/caravan" "$d/results.csv"
```

## Follow-up: global batch-aware construction order

Keep the unchanged farthest-first caravan fallback. Both experimental
arms try eight more complete caravan routes, with the same support-hop
BFS, same carry cap, and exact routes after each pickup. They vary the
start selection using the same seeded noise. The batch-aware arm also
rewards up to three close, not-much-farther same-color slimes and
penalizes clearing likely support for other local colors. This changes
global processing order, not the pickup path rules.

| Fresh cases | Frozen moves ↓ | Randomized order ↓ | Batch priority ↓ |
|---|---:|---:|---:|
| Dev (30) | 436.267 | 431.467 | 435.400 |
| Holdout (30) | 433.833 | 429.800 | 432.533 |

Batch priority improves on frozen in only 4 holdout cases and ties
26. Against equal trials of randomized order it has 1 win, 16 ties,
13 losses and worse mean. Randomized improvement is also modest;
retain frozen as default and record both arms as optional. All 180
runs completed legally and matched the exact independent score;
maximum observed wall .086292 s. [Logs](benchmark-batch-results.csv).
No final cases were run.

Next test one truly different transport transition, such as two-color
shared movement with explicit parity or preserving a stationary support
until all planned routes using it finish. Do not infer historical gain
from the above result.

```sh
p=problems/ahc072
d=$(mktemp -d)
for mode in 0 1 2; do
  g++ -std=c++23 -O3 -DAHC072_BATCH=$mode "$p/main.cpp" -o "$d/arm$mode"
done
python "$p/benchmark_batches.py" /path/to/gen /path/to/vis \
  "$d/arm0" "$d/arm1" "$d/arm2" "$d/results.csv"
```
