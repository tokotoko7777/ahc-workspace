# Disjoint-anchor retry after a dense-L6/L7 wildcard miss

The predecessor is `ddaea58`: native complete-geometry retry plus original complete cycles, five-overlap full
reconstruction, alternate complete-anchor rescue, and the original wildcard
fallback. The optional retry only starts after the original wildcard scan
ran but did not improve ordinary construction. If primary wildcard geometry
was accepted, or full geometry entered, the existing path is not replaced.

Use known-length ranks 6--10 rather than the original wildcard ranks 1--5.
Each row offset pair contributes up to three candidate phase placements.
Each ID triple mixes its highest raw match and strongest relative phase
surprise before round-robin fourth-row checks. Exact rejection of every known
conflict, bipartite distinct-column checks, and independent corroboration
using unused partial lines are retained. This is a different identity search
rather than indiscriminately widening the first shortlist. Recovered seeds
are considered only by exact score and an unsuccessful retry restores the
previous random engine; unsuccessful retries still consume annealer time.

The last fill can start before 2240ms; the solver retains the established
2780-ms deadline. Logs report a separate `disjoint_` line.

```sh
python3 scripts/bundle.py problems/ahc004/experiments/wild_disjoint_retry/submission_entry.cpp -o submission.single.cpp
g++ -std=c++23 submission.single.cpp -o submission
```

Synthetic-only: `e2e.py` checks valid boards and computes actual covered-read
counts from both compiled outputs. Twenty fixed independent seeds comprise
five each at L6 M720/M800 and L7 M500/M680. The independently generated hidden
boards are not supplied to either binary. From this experiment directory:
```sh
PYTHONHASHSEED=0 python3 e2e.py /path/to/ddaea58_parent /path/to/submission
```

In these fresh matched synthetic fixtures, four formerly incomplete cases
reached full coverage through a selected disjoint seed. Coverage-normalized
improvement (not an official score, ignoring any full-board empty bonus)
was approximately 174.6 million summed over the 20 fixtures. Previous paths
still exhibit the normal independent wall-clock variability. No official
score was measured for this branch here.

The twenty paired synthetic results archived here originally compared the
isolated retry against `eb39b4f`; they are not quantitative results for
the integrated branch. The complete-geometry native retry and wildcard
disjoint retry enter on mutually exclusive conditions.
