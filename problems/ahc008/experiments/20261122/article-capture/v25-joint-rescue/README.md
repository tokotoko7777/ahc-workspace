# v25 M5 joint two-ended rail + three-worker rescue

Pinned v25 source. No change to main geometry or any pre-release commands. At actual early fallback t160–204 only, and only with all five in one component with >2 reachable animals, jointly attempt a real rail pair and an independently generated complementary three-worker pocket. Existing global-clear, lastcap, dog/seam and final-gate/terminal priority remain.

Order: for real prepared open rails, their exterior stances at actual path ETA ≤3 and distinct guards, rank at most three different (tube, animal, pair) hypotheses by a cheap existing observed-graph single-animal forecast, with truly legal immediate captures first. Starting at each hypothesis, call original static/mincut pocket selector with only the remaining three workers, the rail animal masked out, and the rail gates/body/stances/actual next steps protected. The winning pocket must genuinely isolate a different current reachable animal on the projected board, leave all five current people and protected rail access connected, have 1–8 unresolved walls and retain ≥300/75%. Run original pocket executor from scratch with the actual three-worker ownership mask, then attach only the two certified rail commands. Validate the full union of first-step commands against actual walls, people, animals, connectivity, retained area, and exact rail containment when immediately closing. On failure restore the exact original rescue path; this is not a post-hoc override of old all-five commands.

Simplification: re-enumerate these two missions on each actual state, preserving no private roster, and try only three hypotheses, not a complete optimal assignment across races. Protect current rail next moves and all physical gate/body/stance cells (plus certified retained-side reachability after projected pocket walls), not a fixed long walking trajectory. Candidate exists only if the pocket crew can do meaningful work this turn. The core selected rail animal is excluded from pocket scoring and certified outside; attempts/intent do not guarantee future paths. Extra search can consume clock budget and affect old late cutoff behavior even if no mission is committed. Record final legal outcomes and control hashes, not only the helper's emitted action counters.

Build: python make.py /path/to/pinned-v25.cpp solve.cpp; g++ -std=c++23 solve.cpp -o solve. --no-joint-rescue retains exact original algorithmic path. Optional AHC008_JOINT_RESCUE_LOG: emitted joint turns, same-turn rail closes, expected legal rail heads, and three-worker pocket wall commands. Replay physical closure and actual isolation must be checked separately (repair/tacticalGate can override the proposal). Unit: g++ -std=c++23 unit.cpp -o unit && ./unit. No official timing was done.

## Paired validation

| Inputs | Score v25 → joint rescue | Full | Full with ≥300 human area | Leftover |
|---|---:|---:|---:|---:|
| Fixed M5 development 30 | 536,821,892 → 603,466,101 | 11 → 13 | 4 → 5 | 79 → 55 |
| Preregistered unused M5 100 | 1,391,420,859 → 1,427,909,355 | 24 → 24 | 6 → 6 | 325 → 307 |

All 260 paired runs legal with official tester and independent playback.
The held input digests match the manifest and are disjoint from prior
M5 screens. On those 100, t161–205 physical four-cell captures 8 → 28,
total captures in that window 120 → 134, and all-turn four-cell captures
657 → 676. Whole-clear flips are ten gains and ten losses, which sum
to a tie rather than zero impact. 49/100 action histories change. Maximum
local tester 0.802 s on both development and held cases (rounded).
This is not a successful promotion: useful full clears did not rise,
and the candidate was not tried on the two general panels. Keep v25.
No grade about the authorized original or historical judge.

For the fixed 100 decompress the seed file and run `holdout/generate.py`
with the official generator. No tuning to this set. Bench harness and
exact historical v25 source in [parent checkpoint](../v25-early-seam/README.md).
