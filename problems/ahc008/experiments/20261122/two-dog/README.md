# AHC008: wait for both dogs before a single four-cell closure

This is a compact capture-only proof for exactly **two** visible dogs. No 300-turn score overlay, hidden targets, RNG access, distributed case tuning, or claim of normal score improvement.

Run the already frozen one-cage geometry and constructive dog bait policy from `../dog-transit`: six board-edge side walls, a six-tile lane with both doors open, two exterior guards and the other humans acting as bait on the far side, at most 75 turns from initial board. Compare only the stopping rule: `mode=0` closes at the first legal occurrence of one or more dogs in the two central positions of the future four-cell enclosure; `mode=4` closes only when **both** dogs are there concurrently and both doors can legally be built. The two central cells can hold one or multiple dogs. No adaptation based on a revealed hidden target or a guessed future position, no repositioned cage or second hunt. Actor geometry uses visible initial positions/types; online decisions use public positions/actions. A target-posterior counter is telemetry only. The fixed geometry and every pre-first-window action are shared between arms; the first legal single-dog window's time was actually identical for all 40 paired runs.

Official Sim applies every wall and movement action under its unmodified v3 rules. On each reported closure, directly check the resulting body flood-fills to exactly four cells with no human after the official update, and that its dog count still matches. All construction was legal. Non-dogs still participate in wall legality and movement, and none were incidentally isolated by the reported closures.

| Fresh exact-two 40 | Close first legal positive | Wait only for both |
| --- | ---: | ---: |
| Cases with a legal single-or-pair window | 40 | 40 |
| Cases closed | 40 | 29 |
| Cases enclosing both dogs | 8 | 29 |
| Total dogs in exact-four confinement | 48 of 80 | 58 of 80 |
| Cases with no closure by cutoff | 0 | 11 |
| Total turns with both exterior guard positions occupied | 823 | 1,614 |
| Total initial wall-phase turns | 536 | 536 |
| Illegal outcomes | 0 | 0 |

The delayed rule converts 21 single captures to double captures, loses the 11 single captures that never became a legal pair, and retains the 8 immediate pairs. Net +10 dogs but an extra **791 guard turns** total (+19.78 per case); 11 cases with no compact confinement at all. Evidence for the exact-four capture axis, not sufficient evidence for any higher normal score.

## Reproduce

Use the official [AHC008 v3 tools](https://atcoder.jp/contests/ahc008/tasks/ahc008_a). Generate `in/` from `seedpool.txt` (Python MT key 717429601312; 300 fresh seeds). The subset is first 40 by generator index with exactly two dogs, selected exclusively by visible species count before simulating. Copy `probe.rs` into a temporary official crate as `src/bin/two_dog_probe.rs`, compile via `cargo build --release --bin two_dog_probe` and run `two_dog_probe subset 75 0 4 > pilot40.csv`. `build.sh` is the exact original relative-layout rustc invocation. The outcome file is per-case counts; no raw trajectories or generated inputs are committed.
