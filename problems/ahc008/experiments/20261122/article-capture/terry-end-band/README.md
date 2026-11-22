# Fixed longitudinal end-band variant, static rejection

One four-row periodic construction block: top 0–3 (nominal room rows 1,3)
or bottom 26–29 (27,29), zero-based; no tested alternate depths. Retire an
original valid room only if its entire original classified footprint lies
inside the band. Omit only planned X adjacent to a retired room but touching
no surviving numbered room or gate. Unmark only retired gates. No baseline
or original source changes, no official tester.

Run from root:
```
g++ -std=gnu++23 ahc008_terry_end_band/geometry.cpp -o /tmp/band-geometry
/tmp/band-geometry > ahc008_terry_end_band/geometry.tsv 2> ahc008_terry_end_band/reasons.txt
python ahc008_terry_end_band/exposure.py > ahc008_terry_end_band/exposure.json
```

Each variant preserves all retained footprints, room sizes and known door
positions (IDs compared by a bijection), main exterior aisle cells and the
open-gate partition. With virtual gates closed, all ten connect previously
distinct aisle-bearing exterior components. No surviving room component
is split or merged. The strict unchanged-connectivity criterion fails;
the possible physical corridor shortcut is noted only as a relaxation.

Measured purely on frozen practice: demand zero initial cows, pigs or
rabbits in the full four-row band (initial board has no walls, so all are
accessible); choose top on ties. For this relaxed diagnostic only, 49
qualify and all have at least 12 actual planned X installed by baseline.
45/49 see a slow pet in the omitted-X/retired-gate/retired-room footprint,
44/49 see the same slow pet in that footprint for at least two turns;
7,716 slow-pet turns total, measured before each submitted action.
Of 47 originally all-clear qualifiers, frozen final removal of X plus
retired gates retains clear on 14; X-only also 14. Safe subset adds 312
summed mean cells, 14 cases with at least ten cells. Virtual wall removal
is an illegal operation used only to inspect topology; animal movement
and actual capture outcomes would differ. No live recommendation.

## Temporary deferral (not removal)

`python ahc008_terry_end_band/deferral.py` replays exact all100 independently
with the same zero-initial-slow side selection. The monitored region is
the union of nominal retired rooms, gates and exclusive X, intersected
with actual open cells on each turn. Alarm at the first actionable
pre-motion state (before first full isolation) with **any** pet reachable
from any actual human and at most three open-board steps from the
monitored footprint. No post-isolation visits count.

Of the 49 eligible and 51 with no initial side, alarms are 45 at t0,
three at t1–59, and one at t60–99. All 49 therefore trigger before
t100. Although there are 1068 nominal exclusive X across the selected
cases, the sum of never-triggered X by first full isolation or terminal
is **zero** for both nominal inventory and cages actually installed.
This is a census of original states: it is not a replay of a deferred
constructor and does not assume preserved pet trajectories.
