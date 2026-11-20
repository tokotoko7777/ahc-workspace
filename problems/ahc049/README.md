# AHC049: Durability-Constrained Transport

[Official problem](https://atcoder.jp/contests/ahc049/tasks/ahc049_a) /
[official generator and scorer](https://img.atcoder.jp/ahc049/LDUZCjLO.zip).
2 s, 1024 MiB, at most 16,000 total commands. Move an entire stack one
cell, losing durability per box equal to the cumulative weight above
it. Durability must remain strictly positive. Return to the origin
to deliver the stack. If all boxes are delivered, score is
`16400 - steps`; otherwise score is 400 minus remaining boxes.
The sum over 150 cases decides rank, with no rerun.

Rank 1 was Rafbill per [official results](https://atcoder.jp/contests/ahc049/results/json).
Exact top score is unverified: [standings](https://atcoder.jp/contests/ahc049/standings)
redirects to sign-in, both official case archive CSVs returned HTTP403,
and the expected ALE-Bench ranking ZIP was 404. The 100 official
starter cases have not been shown to be the 150 actual cases.
**Official comparison undetermined.**

## Direct-return reserve and stacked detours

The control starts each empty trip at a farthest unserved box, using
durability to break ties. While returning, add legal boxes in roughly
descending saved-movement benefit per weight along arbitrary monotone
paths. The variant starts at the same chosen box whenever the current
unserved set agrees, but uses a beam of 14 partial stacks and allows up
to four extra movement steps beyond the direct return. Trips are never
compared with later trips using a different set as if their starts
were matched; compare complete end-to-end plans on the same input.

A stack prefix stores an exact durability reserve for going home
immediately. When appending a box, for every lower layer subtract
`(old_weight_above * detour_length) + (new_weight * new_distance_home)`.
Both terms are nonnegative. This makes every accepted prefix provably
finishable by the direct home path; the board does not contain movement
obstacles. Never traverse the exit in the middle of a loaded leg.
Every appended pickup saves positive movement relative to independent
singles. For N=20 the singles already use only 15,599 total commands.
The branch retains the exact greedy solution if the full candidate
does not improve. The candidate switches to greedy trips after its 1.45 s guard.

The [stack reserve helper](../../library/stack-durability-reserve.hpp)
avoids replaying all movement and all higher-layer weights per proposed
box. It uses integer checks, rolls back implicitly on failure, and
takes externally supplied metric distances. Its test directly computes
durability for 2,200 random trajectories, including rejected appends.

| 28 fresh cases each | Greedy mean moves | Beam mean moves | W/T/L | Median saved |
|---|---:|---:|---:|---:|
| Development | 6,426.14 | 5,077.00 | 28/0/0 | 1,299 |
| Untouched holdout | 6,482.50 | 5,167.64 | 28/0/0 | 1,357 |

All 112 outputs matched the official score and independent grid/stack
damage replay. Maximum wall time 0.131841 s, ≤7319 total commands,
minimum validated remaining durability was 1. The branch defaults
to beam. [Paired data](benchmark-results.csv).

Next free the base box choice: choose a small set of distant starting
supports with different durability/cargo potential, compare complete
trip savings and estimated downstream damage, and use the same
exploration work spent on the original fixed-start beam as control.
Also test crossover re-assigning boxes between nearby completed trips
with strict reserve validation. Use fresh seeds, not final cases.

## Reproduction

Let `ATCODER_TOOLS` point to the official tools' `target/release`
after `cargo build --release --locked`. From repository root:

    g++ -std=c++23 -O3 -DAHC049_BEAM=0 problems/ahc049/main.cpp -o /tmp/ahc049-greedy
    g++ -std=c++23 -O3 problems/ahc049/main.cpp -o /tmp/ahc049-beam
    python problems/ahc049/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc049-greedy /tmp/ahc049-beam /tmp/ahc049.csv
    g++ -std=c++23 -O2 tests/stack_durability_reserve_test.cpp -o /tmp/ahc049-reserve-test
    /tmp/ahc049-reserve-test
    python scripts/bundle.py problems/ahc049/main.cpp -o /tmp/ahc049-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc049-single.cpp -o /tmp/ahc049-single

### Alternate support box at constant beam width

Beam width 14 at the furthest box is the fixed control. The variant uses width
7 from that same start and width 7 from a second available box within two
units of the maximum origin distance, choosing the bottom box with greatest
initial durability per unit of remaining carrying distance. It selects the
larger legal reduction against individual-box trips. Maximum retained beam
width per depth remains 14; actual attempted extensions and times are logged
because the remaining sets diverge after the first different trip.

Fresh SHA-256-derived development and holdout sets (28 each, different from the
original experiment) both improved on all cases. Mean moves on development:
5119.21 → 4887.36; holdout: 5126.57 → 4861.64, with median saving 243.
Mean holdout extension checks were 211,110 → 173,933. All 112 official and
independent scores agreed; maximum observed measured wall time was 0.100 s.
Keep the two-anchor variant as branch default. This is not a comparison on
historical cases. Next try exchanging a pickup between neighboring completed
tours, recomputing both capacity reserves before accepting a move reduction.

```sh
p=problems/ahc049
d=$(mktemp -d)
g++ -std=c++23 -O3 -DAHC049_ANCHORS=0 "$p/main.cpp" -o "$d/fixed"
g++ -std=c++23 -O3 "$p/main.cpp" -o "$d/diverse"
python "$p/benchmark_anchors.py" /path/to/gen /path/to/vis \
  "$d/fixed" "$d/diverse" "$d/results.csv"
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/submission"
```
