# AHC008: late rail exposure and a mixed crew

Frozen v24 remains the selected policy unless a change clears the separate
screen in HOLDOUT.md and subsequent confirmation. The exposure audit and
mixed-crew prototype are original C++/Python work, not a copied or executed
TERRY submission.

## Actual replay, five workers

On the same [generated development inputs](../v24-five-worker-rescue/README.md),
all 30 frozen traces checked against official judge replay, the independent
scorer and exact historical action hashes. For the 62 animals still within reach
at the end, measure observed fully prepared two-ended 1×4 tube entries after
the layout switches to rescue and before turn 275:

| Type | Left | Fully prepared tube | Both walls legal if staff were in place | Two distinct guards actually in place | Distinct guards at ≤2 exterior steps |
|---|---:|---:|---:|---:|---:|
| Cow | 10 | 5 | 2 | 0 | 0 |
| Pig | 15 | 5 | 3 | 0 | 0 |
| Rabbit | 14 | 6 | 5 | 0 | 0 |
| Dog | 7 | 6 | 4 | 1 | 2 |
| Cat | 16 | 16 | 16 | 0 | 0 |
| Total | 62 | 38 | 30 | 1 | 2 |

Each column counts distinct animals observed at least once. The last column
includes people already in place. “Legal if staffed” checks current animal
adjacency, actual built walls and no human inside, not whether reaching the
stances would have affected the animal trajectory or legality. The two-step
column considers routes under the current board, not future opportunities.

## Mixed rail prototype

At most one active tube, two people delegated, and all other worker commands
and planned walls preserved. A short forecast (32 legal motion simulations,
max ETA 5) initiates a short lease only for a currently plausible window.
An actual simultaneous closure additionally checks retained human connectivity
and at least 300 cells. Scope is five workers and rescue turns 160–224.

To reproduce from exact v24, use `python mixed/make.py /path/to/solve-v24.cpp mixed/solve.cpp`
then `g++ -std=gnu++23 mixed/solve.cpp -o mixed/solve`.
Use the prior `bench.py` with the same official tester/harness. The 100 seeds
and input SHA-256 for the independent screen are in this directory.
The audit script retains the original workspace paths and the exact
historical input/action hash assertions.

## Verified benchmark outcome

| Same inputs | v24 score → mixed score | Full | Full and useful ≥300 | Residual |
|---|---:|---:|---:|---:|
| Five-worker development 30 | 434,537,087 → 471,571,274 | 10 → 9 | 2 → 3 | 62 → 65 |
| Fixed independent 100, all worker counts | 3,694,637,884 → 3,674,470,004 | 53 → 50 | 43 → 43 | 127 → 149 |
| Five-worker subset of that 100 (13) | 171,722,219 → 151,221,005 | 5 → 2 | 0 → 0 | 19 → 41 |

The developer set had eight true 4-cell captures on intervention turns,
including two cats and three dogs. On independent M=5 the total number
of 4-cell captures rose 87 → 90 and the turn-275 residue improved 83 → 81,
yet final residue sharply worsened. All 260 paired runs passed official
testing and independent replay; all 200 independent input hashes matched
the frozen input manifest. In the independent non-M5 set 86/87 action
hashes matched; the one difference shows 3 vs 5 late MC sample pairs (CPU
reserve), as the new code is gated out. Adoption screen failed. No rerun
of the other two 100s and no main changes. Do not tune against this set.

## The three lost complete cases

Exact replays in `lost-three.json.gz` isolate two different failure modes:

- **0026:** Two early turns spent positioning (no immediate rail capture).
  The same seven animals are eventually isolated together, at state 297
  instead of 261, leaving no time for the last cat and rabbit.
- **0032:** A four-cell cow capture works, and eight fewer animals remain
  at state 275, but an earlier cheap isolate no longer includes a pig.
  The pig is still accessible at the end despite building 42 walls after
  state 160 instead of 32.
- **0089:** An early four-cell pig capture works, with one fewer animal
  left at state 275. However two cows remain accessible even after 56
  post-state-160 walls, versus 46 for the complete baseline. The later
  isolate contains a different collection of pets.

This is not evidence that the two workers directly cancelled wall-build
commands; they did not. Worker positions and animal trajectories
diverge, and late cuts then isolate different animals or arrive later. Do
not equate an extra midgame capture with a reliable full clear. The frozen
v24 late policy and solver choice stay unchanged.
