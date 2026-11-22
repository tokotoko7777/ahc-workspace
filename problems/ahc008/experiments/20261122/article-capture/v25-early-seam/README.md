# AHC008: aisle-local standby + an early short seam

Starts from the separately measured [local-standby arm](../v24-local-standby/README.md).
Keeps the exact main two-ended 1×4 layout, paid actions, and the normal
late fallback. For five workers only when rail control already releases
and during turns 178–204, consider an exact short existing-wall seam
with at least two currently reachable animals separated, ≤10 new walls
and estimated completion within nine turns. The human component chosen
for physical execution must have projected area ≥300 and ≥75% of the
current connected area. An existing area transition after the grace
window still uses the original logic. The projection is not a promise
that the same animals stay inside before wall completion.

GNU C++23 with optimization in the file. Recreate the parent standalone
first, then `python make.py PATH_TO_LOCAL_STANDBY_SOURCE solve.cpp`, verify
the checksum and compile `g++ -std=gnu++23 solve.cpp -o solve`. Identical
paired official tester and independent replay are described in the
previous checkpoint.

## Development and unused five-worker screen

| Set / baseline | Score → early short seam | Full isolation | Full isolation with ≥300 | Final residual |
|---|---:|---:|---:|---:|
| M5 dev30, local standby baseline | 519,557,455 → 536,821,892 | 9 → 11 | 3 → 4 | 73 → 79 |
| New generated M5 100, frozen v24 baseline | 1,291,277,471 → 1,407,647,573 | 22 → 29 | 4 → 5 | 290 → 243 |

All 260 outputs legal with official and independent replay. The 100 generated
fixed inputs had no overlap with the preceding two M5-only holdouts or
the M5-dev30. All 200 input hashes match the official-generator manifest.
Exact four-cell isolation 677 → 687, max local judge 0.667 s.
Raw total at turn 275 is worse (462 → 508) despite the better final
numbers, so do not infer that the improvement is all earlier.
This passed the preregistered subgroup screen only. See `BROAD.md` for the
separate ordinary practice/independent acceptance condition. No result is
a historical judge result or a measurement of TERRY's Rust original.

## Adopted research baseline v25

This exact source passed both predeclared broader confirmation sets and
now replaces v24 as the research baseline, not as a main merge.

| Same inputs | v24 → v25 score | Full isolation | Full isolation with ≥300 | Final residual |
|---|---:|---:|---:|---:|
| Official distributed practice 100 | 3,199,806,134 → 3,294,019,394 | 47 → 47 | 33 → 34 | 117 → 123 |
| Fixed separate general 100 | 3,285,615,612 → 3,358,241,534 | 44 → 45 | 35 → 36 | 148 → 126 |
| M5 in practice (20) | 249,487,325 → 343,589,474 | 5 → 5 | 0 → 1 | 37 → 43 |
| M5 in independent (16) | 145,928,526 → 218,554,448 | 3 → 4 | 0 → 1 | 59 → 37 |

All 400 broad runs passed official tester and independent replay with
input digests. Across both sets, 163/164 non-M5 action hashes match.
The only difference in practice shows clock-driven final rollout pairs
9 → 8 and contributes +111,111 to overall score, not credited to v25.
Subtracting this noise gives +94,102,149 attributable to changed M5
cases. All final full/useful counts are unchanged by that non-M5 case.
Max local tester time is 1.086 s (practice) / 1.044 s (independent);
official time limit is 3 seconds. Worst local timings are not judge times.

To reproduce directly, `g++ -std=gnu++23 solve-v25.cpp -o solve`. No
extra optimization options or arguments. The earlier parent method
`make.py` can reconstruct the same source. Practice cases are the files
from the official tool archive; regenerate separate general cases from
`independent-seeds.txt`, in order. Verify input digests from each CSV
with `audit_broad.py` and run the previously linked pair harness.
The early initial area check ends at 205; a line can finish later under
the original policy. Actual area comes from replay, not its admission floor.

Still below the 6,263,333,336 target and not same-case comparable with
historical judge ranking. Practice residual regressed despite higher
score and better useful full; five-worker capture remains the next
bottleneck to investigate.
