# One-worker exact aisle shortcut: detached PASS

All 100 signed incoming-board transcripts verified by hashes, replayed
with selected a237 stock Normal and the physically recorded map. All
27,500 pre-terminal commands reproduced before snapshots were accepted.
The 19,207 true phase-entry snapshots retained exact prebooked action
vectors, rather than inferring those from final actions. Both the a237
and earlier f83 stock phase implementations are byte-identical.

Raw phase outputs, selected pet IDs and sanitized actions were identical
on all factual entries and 5,000 independent seeded synthetic entries.
Boundary assertions covered ties across distinct move directions, same
pet positions, d=0/30/31, no feasible edge, assigned waits/moves, projected
walls and multi-worker fallback. Zero- and multi-worker graph behavior
remain untouched.

Real column calls: 81,810 total, 24,346 with exactly one eligible worker
(29.76%, present on all100 boards), 25,536 zero-worker, 31,928 multi.
Of singleton calls, 19,583 accepted an edge, 4,763 had no edge, and
1,132 had multiple equal-best pet IDs. Synthetic: 6,625 singleton,
8,208 no-worker and 5,162 multi; 5,578 singleton edges, 1,047 no edge,
1,599 best ties (synthetic IDs do not represent real boards).

Eight paired alternating process-CPU passes timed the complete idle phase:
factual 2.6758739 → 2.4305850 s (−9.17%), synthetic 1.1970858 →
1.0769798 s (−10.03%). Both clear the predefined 5% gate. This measures
only whole idle-phase CPU including the same input-action copy, not
full solver CPU/wall, scoring, or native-clock outcomes. No official
tests, production integration, or repeated timing.
