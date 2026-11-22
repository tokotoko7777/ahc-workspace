# Three frozen worst-loss pairs: change the first fence, not the score weight

This is post-hoc diagnosis of three requested loss cases, not a training set
or comparable population sample. Instrument only tactical planning output
and live deferred-square telemetry. Reproduce all six exact input and
action hashes from archived 100-case rows, official scores and official
replay. Every coordinate/turn below is zero-based.

Earliest major graph event means a wall-only decrease of ≥40 human region
cells or ≥3 reachable pets, measuring pre- and post-wall components at the
same actual end-of-human-phase locations and before pets move. Each omitted
baseline wall was also separately restored on the contemporaneous variant
state, undoing any move/build by the same human, and rejecting illegal
restoration. This single-turn local counterfactual does not reconstruct
the subsequent sequence of walls or animal responses.

| Case | First changed ordinary wall | Original dog fence ready → deferred | Actual planned dog gate wall | First major graph turn | Final human components (area, reachable pets) |
|---|---|---|---|---|---|
| Practice 0005 | t0 worker5 (18,4) instead walks U | 15 → never | t56 → never | 56 → 84 | 9×(116,0)+1×(120,4) → 10×(14,0) |
| Practice 0009 | t1 worker3 (11,25) instead walks R | never → never | t74 → never | 112 → 110 | 5×(120,0) → 5×(712,13) |
| Generated 0059 | t7 worker0 (1,9) instead walks D | 21 → 25 | t55 → never | 55 → 199 | 6×(481,0)+1×(55,5) → 6×(194,1)+1×(9,1) |

All three have changes from t0. The earliest actual wall-state changes are
respectively t0 (omitted wall), t1 (omitted wall), t1 (new compact wall at
(1,2), even though first suppressed ordinary build comes later). All four,
six and three omitted ordinary wall **commands** before t68 were on the
intended dog fence, not on target/target2; some are repeated attempts on
the same cell. Every tested restoration was locally legal, but none alone
changes an immediate human component by ≥40 cells. Each independently
selected square had zero shell cells in the initial dog fence.

- 0005 loses ordinary fence completion and abandons at t70. Ordinary
  dog gate closure had separated most people from 13 pets on turn56. The
  pilot's later ordinary cuts eventually leave everyone clean but at only
  14 cells: 11,683,333→1,555,556.
- 0009 both leave the dog routine at t70 without its special finish.
  Unmodified tactics build the intended gate cell later (t74) and still
  find a five-person 120-cell clean component at t290. Variant never
  closes that old gate and no late comparable separation follows from
  the observed path: 13,333,333→9,657. The old gate's mere build is not
  claimed to cause the t290 finish.
- 0059 both finish the early fence, but the variant is four turns later
  reaching dog-wait phase (25 vs 21), waits until t132 to even start the
  discharge phase, and exits it unsuccessfully at t183. Control closes
  at t55, benefiting six humans. Pilot did get a real four-cell capture,
  and final unique reachable count went five→two, but six humans lose
  a 481-cell clean component. Its last turn separates another human
  into a nine-cell mixed region: 45,836,806→9,309,524.

## Concrete co-design criterion (not implemented)

Choose the main opening fence first and permit an early exact-four
site only when a full two-cell side is **literally the same planned
fence work**, or initial fence selection can tie-break among equally
cheap feasible contours to produce such a side. Any early first-side
build must remain a valid ordinary fence action if the small pen is
abandoned. Preserve the scheduled fence-wall builders and their approach
actions until the shared side is complete; only a currently spare
exterior worker may add the 1–2 remaining walls, and only with an
observed animal and a real legal gate. Explicitly reject compact closures
that block the dog gate, decoy path, or fallback worker access. Otherwise
emit ordinary tactics unchanged. This requires retaining the real dog
opening geometry (the preceding tactical-intent audit logged target
slots only), and should first be checked on broad development for
shared-side prevalence and whether genuine spare actions exist. All
three observed failures had zero common shell work, so the proposal
would decline them by design, not by case IDs. These traces do not prove
that such co-planning would improve the wider distribution.

To reproduce, apply quad-live-pilot.patch, quad-watch-on-pilot.patch,
quad-deferred-on-watch.patch then quad-causal-logger.patch to scratch
tactical source and compile with `g++ -std=c++23`. Use the benchmark
wrapper folder and already archived frozen case tables:

    python quad_causal_trace.py "$HARNESS" "$TESTER" ./instrumented "$PRACTICE" "$FRESH" "$ARCHIVE" > three.json
