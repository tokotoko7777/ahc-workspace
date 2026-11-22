# Exact near-term pairing in the repeated banks: rejected controlled A/B

Keep the first pilot's body-only 3×12 geometry, all setup targets, initial
case-conditioned orientation/offset, the t101 staffing transition and t225
tactical fallback unchanged. Credited direct captures remain exactly four
actual floor cells under transcribed official replay. For all 30 paired
cases require identical geometry and identical action bytes t0–100,
60/60 legal, and every human in the one connected service-lane component
through turn225 in both arms.

Variant: choose disjoint actual worker pairs (up to 10 humans) across
eligible tubes, maximizing exact legally closable pets now first, and
forecast value second. Use actual shortest routes to both gate stances,
at most four turns. Estimate per-pet blocker and central-cell probability
using current wall graph and official uniform passive substep distributions
for cows/pigs/rabbits, and unbiased two-step approximations for dogs and
cats. Credit each anticipated animal to one tube only. If no joint order
qualifies, keep the original lane navigation, avoiding a smoke-test
deadlock where no pairs happened to be nearby initially. Near-term picks
cannot legalize a wall; observed occupancy still decides each actual
seal. No parameter grid.

| Dev30 metric | Prior | Joint |
|---|---:|---:|
| Exact-four direct pets | 133 | 129 |
| Actual paired seals | 127 | 127 |
| Direct species cow/pig/rabbit/dog/cat | 21/24/17/46/25 | 15/21/23/46/24 |
| Unique animals with any legal ready motif window | 378 | 389 |
| Eligible animals never captured by banks | 245 | 260 |
| Staffed / unstaffed legal motif-turns | 128 / 2711 | 127 / 2853 |
| Per-case median of first-window-to-capture latency, then median over capturing cases | 84 | 95 |
| Reachable pets at t225 → final | 302 → 265 | 306 → 276 |
| Human union area at t225, mean | 719.3 | 719.3 |
| Final human union area, mean | 644.2 | 671.8 |
| Official score sum | 69,288,715 | 29,290,060 |
| Max tester seconds | .570 | .699 |

Case improvements/ties/losses for compact counts 10/8/12, for score
11/1/18. Joint approaches are not sufficiently better; rabbit gain
does not outweigh other losses. Case-pet eligibility is measured against
each arm's own trajectory and is not a counterfactual expectation.
Unchanged late tactical operations remove 37 (prior) or 30 (joint)
unique reachable pets, but the big residual gap is already there at
release and cannot be assigned entirely to fallback. Reject, no holdout.

Build scratch tactical with parallel-banks-pilot.patch then
parallel-banks-joint.patch in GNU C++23. The integrated exact selector
uses the team's bank_schedule.hpp algorithm. CLI mode1 and2 select
prior and joint. Run:

    python parallel_banks_joint_bench.py "$HARNESS" "$TESTER" ./tactical ./banks-joint "$CASES" compare.csv
