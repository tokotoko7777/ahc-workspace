# AHC004: fixed-crossing whole-line rebuild

This is an opt-in follow-up to the
[official local 100-case experiments](../local100/README.md), whose fast
placement path was exactly tied across the 40 longest-fragment cases.
There are **no quality benchmark results in this commit**. Only synthetic
unit checks and single-file builds were run; all subsequent quality
comparisons must use only the official downloadable 100 cases. No final
rank equivalence, no submissions, no default change.

`AHC004_LINE_REBUILD` reserves at most 280 ms from cached placement SA
(2,500 ms stop instead of 2,780 ms), starts from its best board, and
conditionally rebuilds one entire row or column while freezing the other
parallel lines. Blank the selected line, then for each position and each
letter compute the incremental distinct-word reward in its perpendicular
line relative to the blank board. The unconstrained fill maximizes this
per-position reward, with incumbent-letter ties. Missing words or words
lost by blanking become cyclic fragments. A width-24, depth-3 beam places
compatible fragments and uses orthogonal reward loss and their covered
weight to shortlist candidates. Each word contributes at most four
placements and each line at most 96 initial fragments. These estimated
rewards may double-count across fragments/lines; only the existing full
torus coverage delta can accept a strict improvement. At most four
shuffled sweeps, stopping after a sweep with no gain. The deadline is
checked between bounded stretches, and the 3-second official cap remains
outside the 2,780 ms search target.

`AHC004_LINE_REBUILD_SA_CONTROL` uses the same 2,500 ms first-stage cutoff,
then spends the reserve on low-temperature ordinary cached-placement
SA. Scores before and after each tail are emitted to stderr, while the
standard `iterations=… covered=…` line reports the first-stage summary.
Clocked runs can have different first-stage states; use exact full official
output scoring, not stderr coverage alone, and report longest-fragment
strata separately.

```sh
python3 scripts/bundle.py problems/ahc004/main.cpp -o /tmp/ahc004-single.cpp
g++ -std=gnu++23 -DAHC004_FAST_PLACEMENT -DAHC004_LINE_REBUILD /tmp/ahc004-single.cpp -o /tmp/rebuild
g++ -std=gnu++23 -DAHC004_FAST_PLACEMENT -DAHC004_LINE_REBUILD_SA_CONTROL /tmp/ahc004-single.cpp -o /tmp/control
g++ -std=gnu++23 tests/ahc004_line_rebuild_test.cpp -o /tmp/ahc004-line-unit
/tmp/ahc004-line-unit
```

The source carries the existing GCC O3 pragma. The synthetic unit test
checks both axes and cyclic orthogonal reward against naive matching,
exact monotonicity, and cached count/window reconstruction after application.

## Optional post-rebuild tail

Use `-DAHC004_LINE_REBUILD` and exactly one of these additional flags:

- `-DAHC004_LINE_BROAD`: broaden pool from 96 to 224 and beam from 24 to
  48, retain up to eight placements for each fragment rather than four,
  alternate fragment-weight and orthogonal-reward biases, and rank by exact
  score at every depth. Even after one unproductive broad sweep the next
  bias is tried, up to eight sweeps or the deadline.
- `-DAHC004_LINE_BROAD_SA_CONTROL`: use remaining time from the exact
  post-narrow incumbent for rebased cached placement SA, at temperatures
  0.10 to 0.012. This differs from the earlier
  `-DAHC004_LINE_REBUILD_SA_CONTROL` (which omits the narrow phase).

Use `-DAHC004_FAST_PLACEMENT` in both arms; optional
`-DAHC004_EXACT_PLACEMENT` can be enabled in both as an identical upstream
setting. With no environment variables, complete runs keep the 2,780 ms
target. Read `line_rebuild` then `line_tail broad/sa` logs for initial/final
scores, deadline/timing, and actual proposals or evaluations.

For an **identical-incumbent** mechanism comparison, run any
`-DAHC004_LINE_REBUILD` executable on the input with
`AHC004_LINE_CHECKPOINT_OUT=/tmp/case.state`. The snapshot contains the
exact post-narrow board, pending time, line permutation, random state, and
input fingerprint. Then run both tail executables on the same official
input with `AHC004_LINE_CHECKPOINT_IN=/tmp/case.state`. Both rebuild the
same caches and get the same saved time **starting at the tail**; compare
solver outputs via the official tool. Do not mistake standalone tail wall
for the normal end-to-end time, and do not use checkpoint files in
production runs. Cold timed prefixes in full independent runs can still
end at different boards, so record this separate diagnostic when needed.
No official comparison was launched for this change.
