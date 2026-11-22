# AHC008: simultaneous irregular two-gate ablations

Not promoted. Frozen v24 remains the baseline on both published 100-case
local suites. This is a separate five-worker development comparison on the
[same 30 inputs and replay harness](../v24-five-worker-rescue/README.md).
All 120 paired executions validated under the official tester and independent
playback, with complete input/action SHA-256 and isolated animal metrics saved.

| Comparator | Score | Full | Full with ≥300 average human area | Animals remaining |
|---|---:|---:|---:|---:|
| v24 control | 434,537,087 | 10 | 2 | 62 |
| Ready two-gate closure | 470,780,101 | 11 | 2 | 61 |
| + One turn of provably safe staging | 469,446,768 | 10 | 2 | 62 |

Ready mode changed four cases, built five legal pairs, and isolated seven
animals in those turns. The newly complete case ended with 106 cells for
humans. The staging arm altered only one case, staged once, and built both
walls on the next turn. It incurred a net score and full-clear loss over
ready mode despite taking an additional actual opportunity. Max local judge
wall clock 0.630 / 0.659 seconds (ready / staged). No holdout attempted.
The official original submitted by Terry was not executed or copied here.

Ready mode checks candidate wall pairs in the actual motion graph only
during five-worker rescue, turns 160–244, once the existing rescue chooses
its baseline command. Workers are adjacent, neither replaces an existing
wall command, and all other actions survive. The pair must capture more
than either single closure and keep all workers together in at least 300
cells and 70% of the original component. Staging may move just one worker
one turn early and have the other wait, with the current animal-to-gate
distances sufficient to prevent animals blocking either gate next turn.
The next turn reruns actual legality checks. These conditions guarantee only
the proposed local retention, not the final area under downstream rescue.

Use the exact v24 source linked from the parent checkpoint above. In either
subdirectory: `python make.py /path/to/solve-v24.cpp solve.cpp` and
`g++ -std=gnu++23 solve.cpp -o solve`. Check the generated source hashes
against `source-sha256.txt`. Use the prior `bench.py` and `manifest.json`
for official scoring. In the second ablation the immediate variant is the
comparator. Telemetry mode 8 marks a paired closure; 9 marks staging.
