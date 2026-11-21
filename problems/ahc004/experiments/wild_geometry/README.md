# Partial-pattern toroidal alignment, isolated pilot

This optional arm runs only if the original or overlap-merged complete-line
method never started its geometry search, and only for detected L7 (longest
read nine), or L6 (longest read eight) with at least 680 words. Ordinary
construction is completed first and retained as a fallback; the established
overall deadline and action validity checks are unchanged.

The four-overlap assembler now exposes partial 20-symbol lines with at least 12
known letters. A 9-symbol signature table builds per-column candidate masks
separately for three known matches and two known matches. All known conflicts
reject a match. The search maintains one width-192 list per anchor-ID triple
and sends their hypotheses in round-robin rank order to the fourth-stage
extension. This avoids spending the entire fourth-stage budget on a single
apparently better anchor-ID triple. The fourth stage distinguishes four known
matches from three and again tests actual bipartite distinctness.

A candidate places four anchors and distinct consistent perpendicular witness
lines. It may then add unused partial lines with at least five independent
agreeing intersections (and no conflicting letter). Packed 60-bit rows and
columns never overwrite known letters with holes. For false-origin screening,
a candidate must demonstrate two of those additional corroborations, or have
at least 17 distinct four-hit columns. Up to 32 unique relative anchor
layouts are screened; the strongest five are completed by the existing
fill routine. Exact score is used to choose a seed before the existing
annealer. All stages have wall time or attempt caps, with the fill start cap at
1850ms. Logging is on the separate `wild_` line.

Reproduction:
```sh
python3 scripts/bundle.py problems/ahc004/experiments/wild_geometry/submission_entry.cpp -o wild.single.cpp
g++ -std=c++23 wild.single.cpp -o wild_solver
```

All results here derive from independently generated **synthetic** cases.
`synth.cpp` sees only reads; `score.py` compares its resulting partial boards
to separately retained ground truth after the run. `e2e.py` just exact-scores
the boards from the candidate and prior complete-line implementation. These
tests supplied no oracle choices to either solver and are not evidence of an
official-distributed score. To reproduce from this experiment directory:
```sh
g++ -std=c++23 synth.cpp -o synth_v2
PYTHONHASHSEED=0 python3 score.py
PYTHONHASHSEED=0 python3 e2e.py /path/to/merged_retry /path/to/wild_solver
```
