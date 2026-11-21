# AHC008: staffed arbitrary-wall closure

Inputs are local development indices 0000–0039, generated with the official v3 generator as recorded in the existing local development corpus. No distributed practice seed or final-system seed was used in this research. No editorial, participant code or submission was used. Base is the local, independently developed `prob.cpp` habitat/forecast policy at 2026-11-22 08:14 local.

The variant removes every *currently legal buildable cell adjacent to an active worker*, flood-contracts the resulting components, and treats only those gate cells as split vertices with finite capacity. Each component contributes its area as a cost to the captured side, human components are fixed on the retained side, and additive pet reward is swept at five magnitudes. The mincut gate set must allow one distinct already-in-position worker per chosen wall (bipartite matching). If no matching, up to two alternate cuts are tried by forbidding one offending gate. Exact hypothetical component score then ranks feasible wall sets. Mode 3 runs after the baseline plan: retains other planned moves/builds when legal, drops moves crossing a new wall, and re-floods the combined result to require a true improvement over the original plan with no split of previously collocated active humans. Calls begin at turn 125 and skip on the existing 1.65-second solver CPU guard.

| Policy | first 12 average | next 28 average | first 40 average |
| --- | ---: | ---: | ---: |
| Frozen habitat/forecast | 15,756,941 | 13,848,269 | 14,420,871 |
| Mincut override all actions (mode 2) | 17,336,303 | 13,880,996 | 14,917,588 |
| Mincut merged with planned actions (mode 3) | 19,186,355 | 14,692,566 | 16,040,702 |
| Mincut limited to idle workers (mode 4) | 12,955,475 | not run | — |
| Mask-union enumeration merged (mode 5) | 17,936,226 | 14,516,761 | 15,542,600 |

The paired win/tie/loss for mode 3 was 6/0/6 on first 12 and 15/2/11 on next 28. The average is sensitive to occasional cages with all pets captured. Coverage checks used the official interactive tester (no illegal actions); tester wall time can include unrelated host load, and is not a solver-only measurement. Mode 3 closure only on the last turn showed one improvement on 40 runs, with no loss as expected; score cannot decrease since both the baseline turn and the combined turn are explicitly evaluated, and subsequent pet movement cannot cross their chosen walls.

Mask-union enumeration observation: all gate vertices touch a human, so any gate left open is reachable by a human. A compressed pet-only component is captured iff every adjacent gate is closed. The alternate enumerates unions of these gate masks, looks for feasible worker assignments and uses the same combined score filter. It is exact for the standalone no-split closure candidate search if its 4,096-state guard is not reached; merged behavior also depends on which eligible maximum matching is chosen and baseline actions retained, so is not globally exact. It failed to improve over the mincut policy on the 40-case evaluation.

Commands from workspace root:

```sh
g++ -std=c++23 -DHAB_DIAG ahc008_flow/prob.cpp -o ahc008_flow/prob
python ahc008_flow/benchprob.py 0 12 125
python ahc008_flow/benchprob.py 3 12 125
python ahc008_flow/benchprob.py 3 28 125 12
python ahc008_flow/bundle.py
g++ -std=c++23 ahc008_flow/submission.cpp -o ahc008_flow/submission
ahc008_focus_assets/tools/target/release/tester ahc008_flow/submission < ahc008_focus_assets/develop/in/0000.txt
```

The standalone `submission.cpp` includes both libraries and defaults to the winning mode 3 command line. Baseline and alternative raw CSVs are saved separately. Relative-path includes in `prob.cpp` use the existing local checkout's versions of BFS and maximum flow.

### Independent 100-case follow-up

Holding the upstream candidate fixed and changing only the overlay setting
(mode 0 to mode 3, start turn 125), the score fell from 1,161,600,000 to
1,014,600,494 on the independent 100 cases (38 wins, 58 losses, 4 ties;
all legal). Average reachable pets fell from 1.42 to 1.02, but mean area
fell from 177.50 to 133.11. Reject the overlay as a replacement despite its
40-case development result. See independent100-flow.csv, which comes from
the official tester with recorded-action playback and independent score
checking.
