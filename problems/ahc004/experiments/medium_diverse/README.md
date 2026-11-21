# Offset-diverse partial-pattern geometry in the lower-density L6 gap

This optional branch starts at `eb39b4f` (original, rescue, five-overlap full
geometry, and wildcard geometry; no four-overlap full-line retry). It only
tries the new path on L6 (longest word eight) and 600--679 reads if full
geometry did not enter and the primary wildcard stage did not run. Existing
successful geometry, high-density L6, and L7 behavior are preserved.

As in the wildcard solver, mask intersections count only known letters and
reject all conflicts. This arm keeps three high-support phases at every
relative row-offset pair and computes a local null from the phase population.
Within each anchor-identity triple, candidates interleave raw exact matching
and relative surprise. A fixed order of offset pairs spreads clock-capped
enumeration. The fourth stage uses packed masks with optimized independent
distinct-column matching. Realized boards need two agreeing additional
partial lines, each with at least five corroborating known intersections
outside the anchor selection; the existing fill routine evaluates the best
boards using exact word score and only replaces a weaker seed. An unsuccessful
experiment retains the ordinary seed and RNG, though it uses some of the
ordinary annealer's budget. The last fill call may start only before 1815 ms;
the predecessor's final 2780-ms cap is unchanged.

```sh
python3 scripts/bundle.py problems/ahc004/experiments/medium_diverse/submission_entry.cpp -o medium.single.cpp
g++ -std=c++23 medium.single.cpp -o medium
```

Synthetic-only: the first twelve fixtures in `synthetic-results.txt` use
fresh independent torus seeds: four each at M600, M640, and M670. Two reached
full coverage only in the candidate. Most misses lost a few covered reads
relative to the control because annealing had less time. The last two cases
are diagnostic chosen fixtures from earlier research and are not included in
the fresh-cases summary. Neither solver was given hidden board information.
No official-distributed results are claimed. From this experiment directory:
```sh
PYTHONHASHSEED=0 python3 e2e.py /path/to/parent /path/to/medium
```
