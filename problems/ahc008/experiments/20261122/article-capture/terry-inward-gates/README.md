# Inward gate screen on faithful practice100 (offline only)

Reproduce with `python ahc008_terry_inward_gates/screen.py`. Requires the
original practice ledger/input and all exact scaffold-audit tapes. It
asserts full action/input SHA and independent replay score for every tape.
No solver or tester execution, no edits to the faithful solver.

Eligible: real normal-mode closed wall on a semantic left/right mouth,
actually built from the outward side, with its next horizontal inward cell
in an actually newly disconnected occupied component. Keep all other
simultaneous actions. Consider one or two tied mouths in each component.
Replace each old wall by a worker position after a hypothetical step onto
that mouth, and place its new wall one cell inward. Reconstruct exact
components: only those removed old gate cells may increase the original
exterior; no existing human floor or connectivity can be lost, no extra pet
can join any person's component, the occupied room must be exactly the old
room minus the new wall(s), and every pet ID in it remains inside. Real
board connectivity catches lateral escape/corner/inter-room conflicts.
Reject target wall on/adjacent to any factual pet or on any projected human.

Instant gain borrows advanced guard positions, so it is spatial only, not
an implementable same-turn action. Stronger next-state compatibility (also
requires instant feasibility) checks same IDs and legal new walls against
actual t+1 animal endpoints. The actual old gate is blocked by then: we
explicitly keep it synthetically open, replace that original build by the
one actor step, keep all other t-command effects, and do NOT apply the
actual t+1 command. Animal endpoints are borrowed from the frozen tape,
not assumed to remain if a gate is left open. No t300 closure. Both are
per-room maxima summed over episodes, not a simultaneously certified
schedule or measured final-score change.

1427 real occupied capture rooms, 1474 heads in them. Absolute ceiling
from the 2431 factual mouths is one newly gained cell per displaced gate.
Instant alternatives yield +1092 summed mean cells across all 100 (998
rooms, max +23 on one board, 94 two-cell rooms). One-turn-compatible
alternatives yield +655 across 98 boards (584 rooms, max +18 on a board,
71 two-cell rooms). By map 0/1/2/3/4, instant / next = 0/0, 285/237,
434/213, 258/139, 115/66 (no map0 boards in this panel). Physically sized
<=4 rooms contribute +812/+411; >=5 contribute only +280/+244. For large
rooms only even the unconditional 357 already-installed mouth count
falls far short of the approximate 1070-cell target. Next-state
all-room sum is a permissive ceiling for these screened endpoint-compatible
witnesses, not a causal upper bound on future variants. No trial proposed.
