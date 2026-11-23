# Narrow-room encounter rates on completed stock maps

See PRESET for the frozen graph model. This is the probability of
entering a gate-safe cell of an eligible named room at a decision
boundary within the next 25 turns, before any such room has actually
been closed. A uniformly selected initially open cell, one animal,
complete original construction, every room open, and instantaneous
guards. The room body is checked by exact flood under its own gate
closures. This is not a deployed capture rate, nor a universal upper
bound after changing the board. Hitting is purely a geometry/motion
diagnostic.

| Map | Actual ≤4 rooms / nominal rooms | Cow any / ≤4 | Pig any / ≤4 | Rabbit any / ≤4 |
|---:|---:|---:|---:|---:|
| 0 | 7 / 53 | 86.5% / 8.1% | 91.8% / 10.6% | 96.8% / 13.7% |
| 1 | 57 / 76 | 81.2% / 49.8% | 88.6% / 59.4% | 95.6% / 70.9% |
| 2 | 70 / 75 | 76.2% / 69.8% | 85.5% / 79.2% | 93.1% / 87.6% |
| 3 | 73 / 75 | 72.8% / 70.2% | 66.3% / 63.6% | 88.6% / 86.5% |
| 4 | 70 / 75 | 74.9% / 69.0% | 83.9% / 78.0% | 92.4% / 87.0% |

Every open cell can eventually reach some eligible narrow target
with the fixed passive step count. The finite-time gaps still matter.
Map 3 has 27 fewer planned X and three more narrow rooms than Map 2,
but those counts overstate pig service: 25-turn narrow access is
about 16 percentage points lower; the lower pig color has 58.2%
mean chance and 14.0% 10th percentile (Map 2 both colors 79.2%
and 53.2%). Map 1 loses 29.1 percentage points on pigs if it ignores
all wider targets. This gives a concrete topology cost behind a
narrow-only or sparse layout, without establishing what to do in
an actually observed capture.

Benchmark comparison covers only cows/pigs/rabbits. In the
atlas, probabilities include an eligible start at t=0; uniform
step over open neighbors, stay only if degree zero. All 100 random
small-graph fixtures compare every one-, two-, and three-step result
at two future horizons with an independent rational path oracle.
No signed animal trajectory, run evaluation, solver or Git main change.
Run `python3 ahc008_terry_passive_opportunity_atlas/atlas.py` from the
research workspace with the earlier catalog and terrain sources as
specified in imports. A factual preclose snapshot test would need to
condition on current walls and pet positions before justifying
even a trial of deferring a wide capture.
