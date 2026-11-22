# Exact-four shell: short watch vs same placement

Freeze the previous pilot's initial choice, three assigned people, route,
shoulder-wall jobs, forecast window and give-up rules for unfinished
construction. Change just the release rule: first ready turn plus at most
three further actions vs old 27 total ready turns (both have original t68
hard stop). This is an independent matched development comparison, with
seeds in quad-watch-fresh30-seeds.txt. It is not a promotion or original
distributed competition result.

Run control and three paired geometry arms. The harness checks identical
chosen geometry/crew metadata, identical full shell-ready turn and identical
action prefix to effective early-release turn on every pair, an assertion
in addition to independent exact replay and official tester agreement.
All 210 legal; maximum observed tester wall among the seven arms .642s.
A read-only side geometry analysis may have overlapped a few late
measurements, so no performance optimization is inferred from timings.

| Shape | Pilot pets, full → short | Diverted commands | Pets per 1,000 diverted | Shoulder builds | Staffed-ready turns | Score, full → short |
|---|---:|---:|---:|---:|---:|---:|
| Line | 6 → 2 | 1702 → 974 | 3.53 → 2.05 | 149 → 149 | 706 → 112 | 395,585,746 → 397,605,434 |
| Square | 7 → 5 | 2088 → 1118 | 3.35 → 4.47 | 141 → 141 | 616 → 103 | 509,740,990 → 457,027,033 |
| L | 4 → 1 | 2290 → 1331 | 1.75 → 0.75 | 177 → 177 | 669 → 105 | 489,717,593 → 444,295,259 |
| Three pilot arms summed | 17 → 8 | 6080 → 3423 | 2.80 → 2.34 | 467 → 467 | 1991 → 320 | Do not sum unrelated models |

Fresh tactical-only control score 516,113,877; 3 final exact-four animals,
159 final reachable pets (5.30 per case), one all-clear. Full→short final
four counts line 7→5, square 8→7, L 4→3. Final reachable totals line
162→149, square 155→158, L 143→170. No cats or dogs were sealed; short
arm counts sum to 6 cows and 2 pigs. Full arm pests sealed sum to 11 cows,
4 pigs and 2 rabbits. The square alone improves worker efficiency but loses
two captures and deteriorates judged score. These are separate matched
inputs across variants, not the previous site's same animal trajectories.
No promotion. The short cap bought only availability, not fewer wall
or positional costs and not generally more return.

On each capture-success case in original dev30, median shoulder walls
were 5/5/5 line/square/L, staffed-ready turns 2/1/1, guard moves 8/12/11,
guard waits 4/8/6, shoulder worker moves 7/21/12, changed commands
31/37/30 (incl. 2/3/3 originally walls). Early guard walks and walls are
material even for the successes that did not wait.

## Early existing side reuse feasibility

On the original tactical dev30, every turn t0–67, enumerate edge-supported
line/square/L placements whose shell exposes at least one complete
straight side with two or more cells (220/112/872 placements). Count that side
if already walled, or if completes with baseline wall actions that turn.
Require still intact body connected to a human prior to closure and no
human currently occupying body or shell. All 90 case–shape reports
have zero complete reused sides. This zero precedes the stricter test of
a pet already inside, ≤3 remaining tasks and near workers. A synthetic
corner square with one side completed confirms positive detection;
official traces had early build commands, just not on required edges.
It is not a claim that no repositioned or newly co-designed separator
can create a useful side: placement was tied to ordinary run's actual
early walls. No second coefficient grid.

A remaining untested route is **co-design the launch barrier**: choose
the same first long perpendicular fence as both the first ordinary
partition of the live board and a full side of one edge-supported square
or L. Assign the last frontier worker immediately after their ordinary
first-side job, then evaluate actual pet-ready short-window eligibility.
Doing another stand-alone shell or waiting for a tactically unrelated
wall to align is not this route. Require low incremental wall count and
early shared stance proof before running a new overlay.

Reproduction after compiling the unchanged baseline separately:

    d=problems/ahc008/experiments/20261122
    patch -p0 < "$d/quad-live-pilot.patch"
    patch -p0 < "$d/quad-watch-on-pilot.patch"
    g++ -std=c++23 problems/ahc008/tactical_gate_submission.cpp -o quad-watch
    python "$d/quad_live_watch_bench.py" "$HARNESS" "$TESTER" ./tactical ./quad-watch "$FRESH_CASES" watch.csv
    python "$d/quad_side_reuse_audit.py" "$HARNESS" "$TESTER" ./tactical "$ORIGINAL_DEV_CASES" reuse.csv
