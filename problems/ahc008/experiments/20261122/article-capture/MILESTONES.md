# AHC008 endgame timing, frozen v7

The baseline goal is full animal separation, not a narrow human refuge:
36/100 in official distributed practice and 44/100 in the independent
generated cohort. This analysis changes no solver actions and has no
extra timed runs. It reads actual first isolation events from the
[replayed paired ledgers](https://github.com/tokotoko7777/ahc-workspace/commit/236d1f9255de15408ee3841ef74876313fa435e1)
and aggregates events *by turn* before recording the remaining count.

| v7 measured waypoint | Official practice100 | Independent generated100 |
|---|---:|---:|
| Final zero / exactly one / exactly two / at least three left | 36 / 25 / 18 / 21 | 44 / 24 / 16 / 16 |
| First exactly one among final-one misses, median observation turn | 280 | 280 |
| Final-one misses that were already exactly one by t250 | 1 / 25 | 3 / 24 |
| Final-one misses reaching exactly one only after t275 | 14 / 25 | 14 / 24 |
| Actual full clears that jumped to zero without a state of one | 7 / 36 | 11 / 44 |

The final-one misses have a median 20 usable turns in exactly-one
states (t300 itself permits no further action). At t250, among boards
already at zero or one, 20/21 practice and 17/20 independent eventually
clear; excluding already clear, 15/16 and 14/17 at exactly one clear.
Neither comparison proves that moving a cutoff advances those states.
They may already have been easier boards. In 13 practice and 12
independent final-one failures there were at least 20 one-animal action
turns: earlier arrival alone does not make the final closure actionable.

Even magically fixing every observed final-one miss while keeping all
other outcomes gives only 61/100 and 68/100. Fixing two-animal misses
as well would give 79/100 and 84/100. This is bookkeeping on fixed
trajectories, neither a performance prediction nor an upper bound on
different future policies. Track earlier real isolation alongside final
clear, retained human area and exact pen size.

`python3 timing_milestones.py paired.csv[.gz] [--cases case_milestones.csv]`
reconstructs case-level time. The original pair archives are in file
halves; join without repeating the header or run one pair summary per half.
