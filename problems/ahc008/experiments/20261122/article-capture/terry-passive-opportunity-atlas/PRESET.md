# Map-wide narrow room first-passage diagnostic

Measure an interpretable limit on one passive animal in each completed
stock map, all mouths open, all intended braces built, guards assumed
instantly available. At each turn boundary an eligible target room has
the animal inside it and not on or adjacent to any mouth. Compare all
eligible rooms versus only rooms with actual isolated body area ≤4.
Once a target is hit count success but do not otherwise change the
board. No other animals, staffing delays, future closures, predictor,
or actual score enters the calculation. This is a static benchmark,
not a guarantee or live policy bound under different topology.

Use the official one-step uniform adjacency transition (stay if isolated).
Compose one, two or three substeps for cow, pig or rabbit, and allow
capture only at the turn boundary. Use all unblocked starting cells,
report unweighted and by both checkerboard colors, unreachable fraction,
and exact finite-horizon recurrence to horizons 10/25/50/100 in binary64.
No cutoff chosen from observed outcomes. Positive evidence can guide a
separate factual audit of waiting for narrow rooms; a diffuse result is
not evidence for changing action timing.
