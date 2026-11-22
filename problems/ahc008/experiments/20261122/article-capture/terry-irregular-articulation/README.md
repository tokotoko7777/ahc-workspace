# Midgame single-wall irregular articulation audit

Reproduce from workspace with
`python ahc008_terry_irregular_articulation/audit.py > ahc008_terry_irregular_articulation/report.json`.
Checks all faithful practice100 input and complete action SHA plus
independent replay score. Uses actual Normal turns 110–225 inclusive
(0-based), existing frozen tapes only, no official tester or solver.

From an otherwise literal-dot worker, enumerate distinct adjacent
non-landmark walls: no configured gates, planned X, or aisle/anchor
cells, and no animal on or orthogonally adjacent to wall, human
start/stock endpoint there, or preexisting/stock-new wall there.
Literal dots are an optimistic superset of genuinely unrelated waits.
Preserve every old nonwait. Require a genuine stock-preaction
articulation separating at least one previously human-accessible animal
before applying other existing actions. Downstream tests would require
actual compact 2–9-cell nonrectangular pen, all people in one exterior
of at least 300, all currently human-accessible configured service
column cells remain reachable, and no lost existing exact stock
capture. Later area credit would be unique by same complete later room
roster, not score or counterfactual trajectory.

There are **17,467** legal deduplicated wall candidates over 100 boards.
**Zero** separate any previously reachable animal in the stock current
graph, even before testing pocket shape/size, area, service or collateral.
Thus zero new actual or otherwise residual IDs and no area substitution.
No live proposal or broader search.
