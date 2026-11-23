# Exact official judge counterfactual: original command suffix is brittle

The one frozen run exited 0, on the older accepted Incoming signed
held100. Stage one matched every input/action/trace hash, every signed
score and full flag, and all 30,000 opponent output lines through the
pinned official tester with a signed-action dispatcher. No solver was
run; the seed remained in the judge. Stage two used the exact judge's
actual response with public-only one-shot omission; no repair.

| Outcome | Count |
|---|---:|
| Exposed one-omission boards | 82/100 |
| Legally completed among exposed | 0/82 |
| Unexposed, identical and legal | 18/18 |
| Baseline vs modified full /100 | 99 → 17 |
| Baseline vs modified useful /100 | 98 → 17 |
| Exposed both-full or legal survivors with floor | 0 |

Zero-based omission t=1…28, median 5. All 82 drifted at t=3…100,
median 23, median 16 turns after omission. Three drifted on the same
turn as omission. Twelve had an observed substep on the omitted cell
before official rejection, t=11…100, median 23.5; eight first contact
events occurred no later than first drift. These observable changes
don't attribute latent target or RNG cause. All observed contacts were
early. Every exposed board reached a first invalid placement at
t=8…105, median 29.5: 56 had only pet-adjacent wall conflicts, 25 only
pet-on-wall conflicts, one had both types among commands in that
action. No move conflicts.

Support threshold passes, proceed threshold fails. There is no legal
exposed mean/both-full delta; the 17 unexposed both-full boards are
identical, not improvement evidence. At most 0.82 mean direct opened
tiles if all exposed scripts could retain the cell, excluding any
indirect effects. Illegal suffixes count against full/useful, without
fabricating a retained floor. This tests only unchanged downstream
commands, not a live solver adaptation or inability to omit fences.
