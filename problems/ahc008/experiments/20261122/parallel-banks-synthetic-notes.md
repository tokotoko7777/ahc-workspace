# Offline-only seeded continuations at narrow-rescue t160

Question: whether a small lookahead can decide between the existing
confirmed early rescue and the existing stay225 policy, based on the same
revealed turn-160 position only. The idea of testing a decision by
late simulation is inspired by the [user-authorized TERRY narrative](https://blog.terry-u16.net/entry/ahc008),
not external submitted code. Used dev30, no new or judge inputs.

On cases where the existing rule actually switches (observed
reachable residual>4), clone every C++ Game policy state at t160. Roll
each of the two unchanged policy modes through t300 on four paired
synthetic scenarios, seeded before opening results:
`0xb16b00b5`, `0x08c0ffee`, `0xd159d00f`, `0x49a008ff`.
No case-dependent tuning, no actual generator seeds or future commands/
pet positions consulted by the predictor. Draws keyed by seed, pet ID,
turn and substep/purpose give each pet a separate counter-based stream,
using the same underlying random cell/target selection variates on both
branches despite later different boards. Exact C/P/R uniform unblocked
substeps; dog follows a persistent reachable human target for one shortest
step then one independent uniform step, resetting on reach/either step and
retargeting as specified, with one-step fallback when no target. Cat
follows a persistent reachable cell with analogous target changes.
**The latent target at t160 is unknown:** sample its normal reachable
reset distribution using the common observed t160 board, not the true
posterior. This approximation and only four paired paths limit inference.

Choose stay only if mean synthetic rounded score is strictly greater;
otherwise keep current. This never actually changes solver stdout.
Each shadow clone has its own clock; observer removes only offline CPU
from the true replay's clock accounting, never valid for a live solver.

Fixtures check exact corridor walk-step parity/ranges, directed target
persistence/reset, dog one-step disconnected fallback, illegal build
adjacent to an animal, illegal simultaneous move into a wall, and
official f64 score on a split board. In live observed official interactions
our separate human phase produced every exact wall/human state, and score
matched the official arithmetic. All 30 current action digests matched
prior narrow baseline, all 24 active stay pairs have an exact shared
t0–159 prefix, all 54 observed runs legal and independently replayed.
All 192 synthetic trajectories also passed the official-compatible
human-phase legality check throughout.

| Used dev30 | Score |
|---|---:|
| Actual narrow rule | 746775771 |
| Always stay225 | 559927122 |
| Four-pair shadow choice (retrospective evaluation) | 742424865 |
| Hindsight exact early/stay oracle (unattainable) | 810847223 |
| Current vs shadow regret to oracle | 64071452 → 68422358 |
| Gross shadow gains/losses | +8806694 / −13157600 |

Switched to stay 4 times, 2 real wins and 2 real losses, net −4,350,906.
Half-seed 0,1 vs 2,3 choices disagree in 9/24 active cases, realized
retrospective totals 721,229,282 / 763,073,436 (diagnostic only; **no**
rerun or best-half picking). Every paired simulated delta and true case
score, choice and timing is in the ledgers. This evidence does not
support selecting by four full rollout means.

The full estimator is also too expensive for live use: active observer
wall median/max 2.153/2.814 s, pure sum of eight shadow CPUs 1.803/2.464 s,
worst single rollout .502 s. 671,233 synthetic budget queries, up to
5,828 in a rollout, none over the current per-clone clock cutoff.
A future bounded heuristic would need an independently verified 25–40
turn prefix value estimator or representative decision features, more
stable target inference/repeat confidence, and the *real* remaining
budget; this full offline exemption is not a deployment technique.
No new cohort, production, threshold retune, or submission.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-synthetic.cpp "$D/parallel-banks-narrow-rescue-research.cpp" "$D/parallel-banks-synthetic.patch"
g++ -std=c++23 /tmp/ahc008-synthetic.cpp -o /tmp/ahc008-synthetic
/tmp/ahc008-synthetic --fixtures
python "$D/parallel_banks_synthetic_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-synthetic "$DEV30" "$D/parallel-banks-empty-guard-dev30.csv" /tmp/mc-cases.csv /tmp/mc-rolls.csv /tmp/mc-results.json
```
