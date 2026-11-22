# v24 M5 aisle-local standby

This replaces only standby for five-worker guards who are unassigned after original construction, lane handoff, immediate rails, and paid forecast pairs, and whose current cell is on an actual service aisle. Other M and off-aisle free actors retain old standby. In-aisle actors never cross or change lanes via the new slice. A currently closed axial path, unusable original opposite gate, unfinished braces, protected bridge or reserved active crossing makes a candidate tube ineligible. Distances to a real animal use the post-paid-wall board. Stance movement remains in the aisle and checks existing/new walls. The assignment is distinct-animal within each lane independently. Adjacent lanes can both stand by for the same animal, opposite guards around the same useful open tube. This is not paid-pair re-ranking, predictive matching de-duplication, added geometry or an extra wall. An in-aisle actor with no local choice holds rather than doing the old cross-aisle park.

Regenerate from pinned exact v24 with Python make.py V24.cpp solve.cpp, build g++ -std=c++23 solve.cpp -o solve. Candidate default enabled; --no-local-standby leaves the original code path. Optional AHC008_LOCAL_STANDBY_LOG reports number of local matched worker-turns and actual issued axial moves. A tiny deterministic unit verifies two opposing guards can match one animal, same-aisle duplicate not matched, paid/off-aisle unaffected and exact off hook. Unit command: g++ -std=c++23 unit.cpp -o unit && ./unit. No official test or private source included.

## Verified results

Same pairs within each set; every output passed the official tester and
independent deterministic playback. The independent 100 is a fresh
five-worker-only stratified generated set; all 200 run hashes matched the
preregistered input manifest. It is neither official distributed 100
nor historical judge input.

| Set | Score v24 → local standby | Whole | Whole with ≥300 cells | Final residual |
|---|---:|---:|---:|---:|
| Development M5 30 | 434,537,087 → 519,557,455 | 10 → 9 | 2 → 3 | 62 → 73 |
| Fresh M5 100 | 1,243,610,972 → 1,168,476,253 | 25 → 31 | 5 → 4 | 333 → 251 |

Held true four-cell isolates rose from 637 to 669, with 23 new and 17 lost
whole clears. Max held local judge wall clock 0.646s. All actions changed,
even though paid actions are initially preserved at each shared observed
state; later outcomes diverge. The candidate failed the score/useful-full
gate. It is saved as a research arm, not a replacement for v24. Do not
tune after observing the held set. The only allowed third-party submission
was inspected statically; nothing here gives its actual capture rate.

Use the paired harness on the [original dev30](../v24-five-worker-rescue/README.md).
For the held set, decompress `holdout/candidate-seeds.txt.gz` in place then
run `python holdout/generate.py PATH_TO_OFFICIAL_GEN`; verify the ordered
input manifest. Full captures with means ≥300 are counted at state 300.

## Exact area replay

`full_audit.py` reproduced exact official commands and scores on the 40
full-clear flips and three further useful-area flips (86 reconstructions).
For the 23 cases newly fully isolated, only three retain 300 average
human cells. The other 20 first fall below 300 on an ordinary rescue
transition (mode 4), with at least one animal isolated by that action.
In 14 of those 20, the area drop and the last animal isolation are the
same transition; 17 happen at state 270 or later. This is not post-clear
wall overbuilding: omitting the decisive action could also forfeit the
clear. The experiment distinguishes counts from physical area and
suggests moving more actual capture into earlier stages.
