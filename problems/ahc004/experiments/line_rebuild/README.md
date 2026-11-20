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
