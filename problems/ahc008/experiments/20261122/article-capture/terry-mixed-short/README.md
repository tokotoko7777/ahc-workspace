# Mixed short-body map2 replacement fixture (not solver)

Fixed aisle columns 3/11/19/27, trap rows 3..27 odd. Each of the three
inter-aisle spans has two opposing 2-body one-mouth rooms sharing a cap,
except rows 7/15/23 where it has a 5-body two-mouth room. Left margin has
a 2-body one-mouth room on each trap row; right margin unused. All
dimensions in zero-based original core coordinates. No parameter sweep
or baseline changes. To permit stock same-row cap work, retain original
mod-4 sweep markers and additionally mark the needed odd cap rows.

```
g++ -std=gnu++23 -O2 ahc008_terry_mixed_short/unit.cpp -o /tmp/terry-mixed-unit
/tmp/terry-mixed-unit > ahc008_terry_mixed_short/fixture.txt
g++ -std=gnu++23 -O2 ahc008_terry_mixed_short/area.cpp -o /tmp/terry-mixed-area
/tmp/terry-mixed-area > ahc008_terry_mixed_short/area.txt
```

82 correct room bodies (73 singles and 9 pairs), 91 gates, 244 planned X.
Independent floods and exact dynamic classify agree on all 82 body
footprints, size including gates, and gate roles. Opening any one gate
connects its corresponding body to actual exterior. All-closed exterior
keeps one connected road component. There are 18 physically isolated
one-cell holes between cap rows, not extra numbered rooms. Open-gate
reachable floor 638 vs stock 640 despite 16 fewer X; complete-seal
road floor 356 vs stock four disjoint 60-cell areas. Static 200 draws
of 20 random physical gates each preserve one road component and show
median 584 vs stock 600; random gate count is not equal capture count
and bears no score inference.

Every route assignment covers 244/244 distinct X in 36 deterministic
uniform starting formations, but 56–74 assigned jobs duplicate some
same X and 18 scheduled work stances are themselves future X, with
possible scheduling conflict. Relative stock median nominal makespan
M8/M9/M10 +20/+18/+17, pooled +18, exceeds +15 preregistered ceiling.
Actual pet blocking and construction handoff not tested. Empty-grid
nearest safe interior Manhattan distance mean 2.17, max 8; worst 3
along left border vs 8 on right border. Independent single-mouth work
may lower concurrent actor requirements, but the right margin and
five-body pair rows still require coverage, and slow dry setup risks
eliminating that advantage. No integration, scoring or live test.

## Single final-open construction-control follow-up

`route.cpp` uses a different complete constructor only in a no-animal
fixture. Assign each X once to a canonical adjacent stance in the
final-open main component: minimum actual distance to a service aisle,
favoring a multi-wall stance on ties. Traverse stance clusters in
aisle-group/Z-row snake order using four fixed mirror combinations.
Optimize contiguous ownership and either interval direction with exact
BFS approach/travel and build time. Evacuate stranded/future-wall initial
starts before the first build stage (at most two actual cardinal steps).
Then simulate legal commands with exact acknowledgements; no constructor
ever walks onto a future X or an orphan. Stock uses its actual literal
crew and same 36 empty boards. Both fill 100% on all 36 starts. Paired
new minus stock actual makespan medians M8/M9/M10 **+45/+40.5/+38**,
pooled **+40**. The method meets geometric/no-stranding obligations
but fails the <+15 time limit; no smaller or altered layout attempted.

The area control now includes 200 uniform 15-room draws where all doors
of each selected room are closed. All 200 retain one road component
for either layout; mean 560.49→585.78, median 560→585. Equal physical
20-gate draws instead favor stock (mean 599.55→583.71, median 600→584).
These are different physical capacities and neither measures actual
animals or score. No live fixture promoted.
