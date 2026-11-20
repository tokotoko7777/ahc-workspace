# AHC046: Skating with Blocks

[Official statement](https://atcoder.jp/contests/ahc046/tasks/ahc046_a) and
[official generator / scorer](https://img.atcoder.jp/ahc046/EuNd3uow.zip).
2 s, 1024 MiB. Visit 39 distinct targets in order using move, slide
or adjacent-block toggle; sliding through a target is not a visit.
Maximum 1600 moves. Completing all gives `1640 - actions`; otherwise
the score is the visited count plus one. Final absolute sum of 150
cases, no post-contest rerun.

Final #1 **220,494**, Rafbill by
[official results](https://atcoder.jp/contests/ahc046/results/json).
Direct case CSV unavailable; the score is from AtCoder's rank/score
CSV in the [ALE-Bench archive](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc046.zip),
LFS-verified SHA-256
`fdca36f73112443dd16d49be36e7c7266ad7f6d7d0f1ef07d3ce0538f7dc100a`.
The 100 tool starter inputs are not established as the 150 contest
inputs. **Official comparison undetermined.**

## Empty control vs persistent stop blocks

Both compute the same empty-grid incumbent by exact BFS between
consecutive targets (one step and slide edges, cost1 each).

The variant considers placing one stop adjacent to either the current
or next target, or removing an existing stop. Do not place blocks on
targets. For each candidate toggle, first route to its neighboring cell
in the old layout, then try the same onward path in the new layout.
Rank by the exact total actions to reach both of the next two targets,
including the toggle; keep a toggle only if that two-leg cost falls.
The block remains for later legs. A time limit discontinues further
toggle exploration at 1.45s. If any altered route becomes impossible
or globally fails to improve, return the empty incumbent.

Repeated single-cell ray scans were avoidable with a generic
[static slide grid](../../library/static-slide-grid.hpp) that builds all
slide/move endpoints with four sweeps of the grid. Both searches use it.
[1,800 synthetic obstacle tests](../../tests/static_slide_grid_test.cpp)
replay each ray naively.

| 30 fresh cases each | Empty mean turns | Stops mean turns | W/T/L | Median saved |
|---|---:|---:|---:|---:|
| Development | 277.367 | 233.633 | 30/0/0 | 47 |
| Untouched holdout | 277.400 | 238.100 | 30/0/0 | 38 |

The untouched candidate average was 4.633 block toggles. All 120
outputs checked every ordered target and matched the official score
via independent action replay. Max measured time 0.059997s. The branch
defaults to persistent blocks. [Paired data](benchmark-results.csv).

Next introduce more candidate stop cells along the line between
targets and naturally reachable approaches, not only target-adjacent
cells. Use a beam across consecutive legs to preserve persistent
stops with benefits beyond one future target. Keep direct replay
and the empty-grid legal incumbent.

## Reproduction

From the repository root, build the official tools with
`cargo build --release --locked` and set `ATCODER_TOOLS` to their
`target/release` directory.

    g++ -std=c++23 -O3 -DAHC046_BLOCK=0 problems/ahc046/main.cpp -o /tmp/ahc046-empty
    g++ -std=c++23 -O3 -DAHC046_CROSS=0 problems/ahc046/main.cpp -o /tmp/ahc046-stops
    python problems/ahc046/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc046-empty /tmp/ahc046-stops /tmp/ahc046.csv
    g++ -std=c++23 -O2 tests/static_slide_grid_test.cpp -o /tmp/ahc046-slide-test
    /tmp/ahc046-slide-test
    python scripts/bundle.py problems/ahc046/main.cpp -o /tmp/ahc046-single.cpp
    g++ -std=c++23 -O3 /tmp/ahc046-single.cpp -o /tmp/ahc046-single

## Across-leg beam

After preserving the same complete two-leg incumbent, examine up to
eight different persistent block maps after each visited target.
For a given map, the cheapest path suffix onto the next target uses
at most one toggle. Compare options with paid length plus an exact
untoggled distance to the following target, group identical maps
at each stage, and keep the best completed route. No new block
candidate cells are added relative to the first search.

The comparison control consumes a similar budget by running eight
single-state greedy itineraries with differing future-distance weights.
It uses identical candidate enumeration and the same initial complete
fallback, varying only how well competing block configurations survive.

| 30 fresh cases each | Prior mean turns | 8 greedy restarts | Beam8 |
|---|---:|---:|---:|
| Development | 236.233 | 226.167 | 217.967 |
| Untouched holdout | 239.400 | 227.933 | 217.600 |

On holdout the beam versus the eight restarts went 29/1/0 (W/T/L),
median 10.5 turns saved. Mean candidate toggles evaluated were 3377 vs
3484.6; median wall0.120 s vs0.132 s. This approximates, not forces,
equal wall work. All 180 outputs were independently replayed without
violation, stopping on all 39 targets, and matched the official score.
Worst measured wall 0.371476s. Branch defaults to beam. [Data](cross-results.csv).

Next allow several more re-usable block positions, e.g. one past
intersections of approach lanes and upcoming checkpoints. Test them
at equal candidate-evaluation quotas; the best current itinerary is
always a legal fallback.

    g++ -std=c++23 -O3 problems/ahc046/main.cpp -o /tmp/ahc046-cross
    g++ -std=c++23 -O3 -DAHC046_CROSS=2 problems/ahc046/main.cpp -o /tmp/ahc046-multi
    python problems/ahc046/benchmark.py "$ATCODER_TOOLS/gen" "$ATCODER_TOOLS/vis" /tmp/ahc046-stops /tmp/ahc046-cross /tmp/ahc046-cross.csv --multistart /tmp/ahc046-multi
