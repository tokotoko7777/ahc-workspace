# Short-horizon stochastic locking audit (cow/pig)

Use the 30 dev inputs listed in collective-capture-dev30-seeds.txt.
Freeze existing capture4 and score-leading tactical actions using official
tester v3, then replay the exact actions and require byte-identical response
verification and independent score agreement. No candidate policy ran
against the tester; baseline traces supplied the current states only, then
were independently used to check whether the same animal got a compact pen.

Inspect all 201 turns 40 through 240. Enumerate rectangles of bounding area
4–24 and clip to the pet's actual passable connected component within the
rectangle. Require 4–24 remaining pen cells, 1–3 exactly computed pending
wall cells, no current human in pen or required wall, and target reachable
from a human before closure. Shapes with the same real interior and pending
walls are deduplicated. The DP additionally requires one target only, all
gates with an exterior stance reachable by a nearby human in ≤2 turns,
and up to two workers selected among the three best coverage sets. This
does not cover all irregular shapes, more than two workers, or multi-target
locks. It deliberately excludes arrangements whose guard must enter and
cross the future pen; other workers hold position in the local model.

Each worker may wait, build an adjacent legal frontier cell, or make a legal
step closer to an unresolved gate stance. Doors shrink monotonically.
The controller observes the result each turn and can retain any still-legal
wall action. For the chosen species, use each official cardinal step
equiprobably among currently passable tiles, once for cow and twice for
pig (including pig leaving and returning within one phase). A pet ending
a phase outside the proposed region is an absorbing escape. This makes the
DP exact for the specified local action set and target distribution up to
floating rounding, not an exact model of all humans and pets on the board.
The target-only number blocks a wall if nearby non-targets currently
prohibit it, but ignores their future movement. The robust number permits
a build during a turn only if no other pet can get within one cell of the
gate in that many turns, using its official maximum speed and the original
passable board as a conservative reachability superset. This intentionally
understates chances under more adaptive multi-pet control.

| Missing walls | Geometry snapshots, tactical | Evaluated shape–pet–turns | Robust ≥0.8 in 2 / 3 / 4 turns | H4 robust ≥.8 already compact by +4 in baseline | By turn 300 |
|---|---:|---:|---:|---:|---:|
| 1 | 144 | 140 | 115 / 122 / 123 | 32/123 | 123/123 |
| 2 | 1321 | 53 | 4 / 8 / 10 | 10/10 | 10/10 |
| 3 | 3285 | 41 | 0 / 2 / 5 | 4/5 | 4/5 |

Capture4 supplied zero shape snapshots before guard checks. Tactical had
234 evaluated opportunities across repeated pet/turns; 62 one-wall states
are immediately robust. The 123 high-probability one-wall states correspond
to 14 unique case–pet IDs, all eventually compact in control; four might
be accelerated because at least one considered state does not close it within
four baseline turns. None of the two-wall witnesses opens a new compact
target within or after four turns. Most of the two-/three-wall geometries
fail near-worker / valid outside stance screening; among the 41 surviving
3-wall states, 29 require a two-step walk for at least one required stance.

One pig in dev case 0002, zero-based turn 104, is a genuine compaction
opportunity: exact reachability would give a five-cell pen. The three
pending gate coordinates are (3,6), (6,3), (7,4), the pig starts (5,5);
two actual guardians can close (3,6) and (7,4) immediately from their
current stances. An action policy then waits/reacts for the last gate.
Probabilities for 1/2/3/4 turns are 0 / 0.75 / 0.8125 / 0.84375,
identical for optimistic/robust in this witness. Original baseline never
puts this pig in a compact pen and leaves it in a 247-cell pet-only
component. So this is novel compaction, not a previously unisolated animal.
See lock-novel-witness0002.json for support, guards and initial actions.

Reject as a strategic overlay on either frozen baseline: one potentially
new compact pen across 30 cases, and no evidence it is free of interference
with other worker tasks. No score/holdout was run. An untested way to
raise candidate frequency is **shared-mouth parallel caps**: find two
neighboring live animal-bearing depressions along the existing wall of
a moving separator. Assign two existing boundary builders to cap the
adjacent 2–3-cell gaps; if the pair cannot cover both gaps within a two-turn
shared tour, continue their original job. Pick the pair and require current
animals before committing to either cap. This uses a joint construction
opportunity already on a worker path, not stationary staffing at a future
arrival site, or pursuing a lone animal into a fresh enclosure. First audit
number of naturally co-occurring depressions plus whether a shared set of
workers can actually cap either side. Do not build if only the same
already-finished captures are observed.

Reproduction from repo root (HARNESS has benchmark.py and benchmark_cases.py):

    d=problems/ahc008/experiments/20261122
    g++ -std=c++23 problems/ahc008/tactical_gate_submission.cpp -o tactical
    g++ -std=c++23 problems/ahc008/capture_first_submission.cpp -o cap4
    python "$d/stochastic_lock_audit.py" "$HARNESS" "$TESTER" ./tactical "$CASES" geom.csv > states.csv
    python "$d/stochastic_lock_witness.py" "$HARNESS" "$TESTER" ./tactical "$CASES" states.csv > witness.csv
    (cd "$d" && python -m unittest test_stochastic_lock_audit)

Repeat the first audit with ./cap4 for the subtraction-free grid control.
