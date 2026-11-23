# Small-bipartite Hall deficiency (library candidate)

Each task supplies a bitmask of eligible workers. With at most 16
workers, ahc::find_hall_deficiency returns either no deficiency on
this exact bipartite graph or a concrete group of tasks that cannot be
assigned distinct workers. It does not construct a matching. See
example.cpp for use. Zero workers and zero tasks are accepted.
Worker count outside [0,16] or any undeclared worker bit is rejected,
even if an earlier task already proves infeasibility.

For worker subset S, the subset-zeta transform counts tasks whose
eligible-worker masks are contained in S. More than |S| such tasks is a
Hall obstruction, and every Hall obstruction implies one such subset
(take the union of its task neighborhoods). Select minimum cardinality
S, breaking ties by numeric mask, then take the first |S|+1 qualifying
tasks in original order. Returned worker_neighbors is the exact union
for the returned task group; indices are zero-based. Complexity for
W workers and T tasks is O(T + W*2^W) time and O(2^W) scratch, with one
counter per subset. Counter entries sum disjoint input categories and
are bounded by the input span size. Use ordinary bipartite matching
when W is larger or you need an assignment rather than a certificate.

In a planning problem, if edges represent only optimistic eligibility
(e.g. travel before future walls), deficiency rules out exact
feasibility as long as all real edges occur in the optimistic graph.
No deficiency then says only that this necessary test passed, not
that true routes or simultaneous actions are feasible. Do not apply
the same exclusion to a subset found deficient only after adding
specific walls that might be removed in a different plan.

Derived from the independent [detached Hall staffing audit](../RESULT.md). The useful certificate is separate
from AHC008 solver adoption: bounded Hall recuts produced five
staffable snapshots on four boards but only 18 reclaimed cells,
below the predeclared 50. The live solver was not changed.

## Reproduce the verified tests

    g++ -std=c++23 -O2 -Wall -Wextra -pedantic test.cpp -o /tmp/hall_test
    /tmp/hall_test
    g++ -std=c++23 -O2 -Wall -Wextra -pedantic example.cpp -o /tmp/hall_example
    /tmp/hall_example

The normal and UBSan tests both passed; see [verification results](VERIFIED.md). To run with UBSan, add `-fsanitize=undefined -fno-sanitize-recover=all`. The locked original 5,053 exhaustive
and 8,000 random cases compare with an independent brute assigner,
plus validation, boundary counts and deterministic witness checks.
