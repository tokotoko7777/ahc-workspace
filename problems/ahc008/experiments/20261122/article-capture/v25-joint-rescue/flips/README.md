Run from workspace:
```
python ahc008_v25_joint_rescue/flip_audit/audit.py ahc008_v25_joint_rescue/fresh100.csv --select
python ahc008_v25_joint_rescue/flip_audit/audit.py ahc008_v25_joint_rescue/fresh100.csv --inputs CASES --tapes TAPES > flips.jsonl
```

Reads existing tapes only: `TAPES/{baseline,article}/0000.actions` is the exact byte stream from the official tester, and `0000.trace` is the 600-line alternating action/official reply file created by `benchmark_cases.py --playback ACTIONS TRACE`. Inputs/action SHA256, transcript legality and recomputed final score/area are asserted against the frozen ledger. If CPU-clock variability prevents reproducing a historical action digest, do not relax the assertion or silently replace the frozen result; report tape unavailable. No old tapes were found under the joint variant directory.

20 raw flips plus one common-clear 300-area crossing, 0087 (separate from raw). For each raw flip, use every factual winning-arm action first isolating a pet still reachable at the losing finish. The useful-only row instead audits its last clearing action as context, not a claim that this action caused the full area gap. `WWW` is winning wall/animal/person state, `LWW` swaps walls only, `WLW` animals only, `WWL` people only, and `LLL` is the losing snapshot. The other combinations expose interactions. Environment invalidity is its own category (do not assign cause from impossible swaps). Each valid geometry reports already/newly isolated IDs, connected human exterior, score *if stopped in that snapshot*, and pre/post area. Mandate legality is separate: reject any new target occupied by a human/pet; check all pet adjacency; compute minimum exterior bottleneck walk with distinct real builders, before one further build action. Builders route on the post-cut grid so cannot use a proposed wall as an intermediate step. This excludes otherwise feasible serial assignments and is not proof a future target remains legal or that the move preserves ordinary assignments.

Interpret case-wise and at first decisive event, with later events as corroboration. A valid LWW loss of physical separation despite WWW positive is wall dependence at the frozen state. WWW positive but WLW/WWL fail with a valid environment indicates an animal/person position dependence; if single swaps pass but combined fail, report an interaction. LLL valid geometry but invalid currently immediate mandate is evidence to inspect action staffing/legality, not just closure geometry. LLL legal/ready demands a separate policy-execution review on the actual tape. Track >=300 and 160/205/225/260/275/300 stopped-score by arm; the historical final official score is always shown separately. Transfers do not simulate reactions and cannot identify intervention causality.

## Verified comparison

All 21 selected pairs (42 tapes) reproduced the frozen input/action
digests and official/independent scores. Some recording attempts were
discarded when fast/slow clock checks changed the action digest; no
substitute trajectory was used. Consider the first decisive event per
raw-flip board (20), and keep the one area-only flip separate.

| First decisive event | Ten v25 clears lost by joint variant | Ten new clears from joint variant |
|---|---:|---:|
| Winning event directly reproduces factual separation | 10 | 10 |
| Swap only walls: valid but no longer reproduces | 6 | 3 |
| Swap only walls: invalid because observed positions meet mismatched walls | 4 | 6 |
| Swap only walls: still reproduces | 0 | 1 |
| Swap only human positions: valid but no longer reproduces | 7 | 6 |
| Swap only human positions: invalid | 2 | 2 |
| Swap only human positions: still reproduces | 1 | 2 |
| Swap only animal positions: valid but no longer reproduces | 2 | 3 |
| Swap only animal positions: invalid | 3 | 1 |
| Swap only animal positions: still reproduces | 5 | 6 |
| Losing arm wall and positions together can reproduce winning cut | 0 | 0 |

For every lost clear the missed IDs were first isolated by v25 after
turn 260, although joint had at most as many reachable pets by state 225
on 8/10 of those cases. These are structure/compatible-position checks,
not evidence that copying a last command, or making an earlier move,
would cause success. In particular, invalid mixed walls and entity
positions cannot be used as causal evidence. The two arms physically
arrive at different problems late. Gap repair or last-gate staffing alone
cannot reproduce the observed winning cut on these losing snapshots.
