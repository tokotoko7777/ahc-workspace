# AHC034: Leveling with a Dump Truck

[Official statement](https://atcoder.jp/contests/ahc034/tasks/ahc034_a) and
[generator / scorer](https://img.atcoder.jp/ahc034/vImT4eac.zip). 2 seconds,
1024 MiB, at most 100,000 commands. Maximum score is sought: on each of
150 contest cases, round `1e9 * base / (cost + diff)`, sum these scores;
`base` is original absolute height sum and `diff` is final-height penalty.
There is no post-contest rerun. These solutions leave every height zero.

Final #1 **9,486,350,384**, yosupo by
[official results](https://atcoder.jp/contests/ahc034/results/json). The
official per-case archive endpoints returned HTTP 403; score cross-reference
is the rank/score CSV supplied by AtCoder in the
[ALE-Bench archive](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc034.zip),
SHA-256 matching LFS pointer
`a6c2666e596dee5a743d80cfcfac8cf1d90d80696a2c11acfd4891b7467ce889`.
The official generator bundle has 100 starter cases, not proven identical to
the 150 contest cases. **Official comparison undetermined.**

## Initial route and exact order edits

Each nonzero cell is served once in a route: pick up the full positive
height or deposit its full deficit. For a candidate snake through all cells,
omit zero-height events, then rotate after a minimum cumulative height.
Because total height is zero, the rotation guarantees that the carried
load is nonnegative. Check eight snakes with the true travel score,
including initial empty travel from (0,0).

Both arms start with that same route. The annealing arm proposes reversals,
swaps, and relocations and rejects any path with a negative-load prefix.
A [load-aware route summary](../../library/load-aware-route.hpp) tracks net
load, min/max prefix, geometric length and load-weighted travel. Restoring
base handling cost is trivial because each cell is served fully once.
The existing [segment tree](../../library/segment-tree.hpp) joins a small
number of range summaries to score the move exactly. It has no existing
in-place all-leaf rebuild call, so the experiment refreshes the exposed
nodes linearly the moment an edit is accepted rather than call O(log n)
point updates or reallocate a tree for each accepted edit. The complexity of
a proposed edit before acceptance is O(log n).

| 24 fresh cases each | Snake mean score | Annealed mean score | W/T/L | Paired travel-cost ratio |
|---|---:|---:|---:|---:|
| Development | 23,160,596.71 | 30,677,647.17 | 23/1/0 | 0.73632 |
| Untouched holdout | 25,131,542.38 | 33,027,861.63 | 24/0/0 | 0.74009 |

All 96 official scores matched full independent replay including load,
terrain, handling and score. Worst measured wall time was 0.159890 s under
the 1.55 s internal guard. Maximum output 2,429 commands. Branch defaults
annealing. [Paired data](benchmark-results.csv) do not use final seeds.

Next broaden the route representation: permit multiple visits to one
source/destination and optimize pickup/drop quantities for a fixed walk.
A time-expanded flow graph has forward arcs with distance costs and
one source or sink supply node per cell, linked to all its visits.
Use exact mass balance and integral flow to recover legal partial service,
then test the trade-off of additional empty movement on fresh instances.
The current prefix summary is still useful for validating a stitched route
or exact work in candidate fixed walks.

## Reproduction

From repository root (`ATCODER_TOOLS` is the extracted official tools'
`target/release` directory, built with `cargo build --release --locked`):

    g++ -std=c++23 -O3 -DAHC034_ANNEAL=0 problems/ahc034/main.cpp -o /tmp/ahc034-snake
    g++ -std=c++23 -O3 problems/ahc034/main.cpp -o /tmp/ahc034-anneal
    python problems/ahc034/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc034-snake /tmp/ahc034-anneal /tmp/ahc034.csv
    g++ -std=c++23 -O2 tests/load_aware_route_test.cpp -o /tmp/ahc034-summary-test
    /tmp/ahc034-summary-test
    python scripts/bundle.py problems/ahc034/main.cpp -o /tmp/ahc034-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc034-single.cpp -o /tmp/ahc034-single

Optional `-DAHC034_VERIFY` compares every accepted move to full route
recomputation, intended only for local diagnostics.
