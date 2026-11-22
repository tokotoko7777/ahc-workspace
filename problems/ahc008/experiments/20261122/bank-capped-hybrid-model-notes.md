# One-ended reserve pens in paired banks: prebuilt feasibility model

Use the same exact 1×4 body-only braces and lane crossbars. In each
bank cap one end of nominal tube (index+bank)%3=1, leaving all other
tubes paired without change. Compare left, right and checker orientation
(no mirror search against a score cohort). Cost +9 caps on the existing
2×13/112 braces or +12 on the existing 3×12/156 braces. Proof by flood:
all open lanes remain connected, and when both ends of every tube close,
all pens are exactly four cells and all remaining exterior is connected.
Closure-safe body expands from two to three cells in already-capped
tubes, because the old wall is no longer part of same-turn legality.

This model recomputes real official passable-neighbor moves (1/2/3 for
cow/pig/rabbit). Fixed exterior dog model exactly propagates target
persistence, shortest first substep, uniform second, and reset if either
reaches the target. Dogs starting outside capped bodies cannot end a
turn deeper than the mouth-adjacent body cell: the first substep follows
a shortest exterior route, and a second uniform substep may enter only
one cell. On the next turn the first substep leads out, so none of
their capped-branch endpoints is legal for closure. Dogs initially
inside when caps provisionally finished are a separate cohort.
Cat not modeled. Starting positions use the same uniform passable-cell
support in both boards, or the same conditional dog subset.

Staffing is *modeled*, not predicted: paired and single gate crew duty
independently sampled at each epoch and held for one or three turns;
the two modes represent transient vs persistent crew availability.
The approximately .11 pair duty is descriptive staffed/observed-legal
old-dev windows, not a skill inferred for single duty; test plausible
single duty at .20–.50 and optimistic limits. Compare after prep,
not at game turn t120. No modeled construction, multiple pets, nearby
blockers, dynamic human targets, policy competition, case score, or
guarantee of legal matched incremental gains.

At horizon 120, 3-bank checker caps, duty pair .11 and single .33:

| Species/tag | Independent per-turn, baseline → hybrid | 3-turn staffing, baseline → hybrid |
|---|---:|---:|
| Cow | .4181 → .4508 | .3216 → .3766 |
| Pig | .4573 → .5207 | .3588 → .4368 |
| Rabbit | .4838 → .5760 | .4003 → .4981 |
| Dog initially outside capped bodies | .4439 → .4142 | .3432 → .3180 |
| Dog initially inside a capped body | .4986 → .6705 | .3829 → .5208 |

Outside-dog capped capture = 0 in all cases; its total reductions just
meet the predeclared ~3 point limit in 3-bank checker (.0297/.0252).
Average C/P/R improvements +6.28/+7.69 percentage points. This clears
the *model* screen (average passive gain at least 3 points at realistic
single duty <=.50, dog loss about 3 or less) but does not prove useful
live cap throughput. For single duty .20, same passive mean improvement
+3.83/+3.85 points. Two-bank checker is similarly positive passively
at q=.33, but outside-dog loss ~3.63/2.93. No promotion from this model.

Numerical tests check normalized transition mass and zero residual plus
absorbed mass loss; sanity tests of zero/full duty and static geometry
also passed. Reproduce with Python and NumPy:
```sh
python bank_capped_hybrid_model.py > hybrid-model.csv
gzip -dc bank-capped-hybrid-model.csv.gz | cmp - hybrid-model.csv
```
