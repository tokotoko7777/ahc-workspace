# Map3 large edge-belt: first pilot rejected; off-aisle handoff isolated

The static verification and frozen construction thresholds passed.
On all 40 randomized no-pet human staffings the two new tasks build
all 20 cells by turn 133 without losing any old retained planned X.
In the actual first paired panel, all 39 map3 variants build all
20 new cells and 67 fewer planned walls, but **none** of the new
mouths closes. The other 61 maps remain byte-identical in actions
and scores.

| 100 fresh generated cases | Accepted | First belt |
|---|---:|---:|
| Entire paired score | 6,108,216,048 | 5,151,170,579 (−15.668%) |
| Entire raw / useful full | 98 / 98 | 79 / 74 |
| Entire final residual | 2 | 45 |
| Affected map3 raw / useful full | 39 / 39 | 20 / 15 |
| Affected map3 score | 2,529,938,271 | 1,572,892,802 |
| Worst local tester wall | 2.78774 s | 2.89781 s |

All 200 true runs validated with the official tester, official
playback and independent replay/scoring. Mean floor fell by 77.7 on
the 20 common full map3 cases even after planned-wall savings.
This implementation is rejected and no holdout or practice test
for it was attempted.

Before reading the paired result, identified a controller conflict:
the original aisle assignment automatically reserves a return
for a guard who stepped from c26 to c27 on the route to a belt
stance. A fixed-motion fixture with one available worker and one
legal target reproduces R,L,R,L and never closes. Defer only a
return relay to c26 with an empty old construction queue, taking
a belt job or restoring the old relay if none. This yields R,R,R,d
and closes on the same fixture. It is a narrow fix, not a score
claim. Retest only on a completely new official dev100. Static
scope and construction are unchanged. Source of first pilot
d1dad06c5fcabb05188ecfacb98aa3ef1a9b6f3c4390f0536ce997bc4c54896b;
fixed-source transform
71c76fba12798b2d7d2b86a9e0963b868eb4f497eb8ce620873b36b3bab03f79.

The companion gzip contains the independent transforms, source fixtures,
input seed and SHA manifest, factual intercepted-pet tally, dry test,
both guard fixture logs, all first-pilot CSVs and exact result report.
No supplied original selector data, generated C++ containing it, or
executable with it is redistributed.
