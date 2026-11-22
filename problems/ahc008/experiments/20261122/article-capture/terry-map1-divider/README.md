# One-X non-gate divider in original map1

Reproduce from workspace root:
```
python ahc008_terry_map1_divider/audit.py > ahc008_terry_map1_divider/report.json
```
No solver, tester or official scoring. Exact configured map1 body masks
and direction labels from the original semantic terrain catalog.

Map1 has 14 nine-body two-mouth rooms: two at each of seven rows
3,7,...,27 (zero-based). Each has two valid divider cells, horizontally
one short of the center, leaving body sizes 2+6 with gate-safe tiles;
center itself strands two singleton wings so does not qualify. Require
original gates unchanged, exact connected halves, an occupied side
sealable on its own, other side connecting to exterior when open, and
the original physical aisle-partition equality in relevant opening and
closure fixtures for each one-wall edit. No other room touched.

All 19 selected practice boards are the faithful actual map1 cases,
with original input/action hashes and independent replay-score checks.
Count only original real Normal closures that create the exact occupied
body; assess both halves and divider occupancy at that factual instant.
Strictly fixed left tie (the 2/6 options are symmetric by geometric
balance), 137 body closures with 155 heads: 22 closures have a pet on the
hypothetical divider (no credit), three have both halves occupied.
Any single occupied half admits a generous reclaimed-area hint of the
empty half plus its unbuilt exterior gate. Sum by separate rooms then
debit one divider X per board per configured room, even if not used.
This is an optimistic scenario for those captured animals, not a
simultaneously feasible terminal deletion or actual causal outcome.

All14 cuts hint +424 gross / +158 net, but break the original connected
aisle graph even with every gate open. Leaving the central connector row
15 untouched (natural geometry safeguard; fixed throughout) allows 12
cuts and restores open/closed aisle partitions, but reduces hint to
+336 gross / +108 net (<150). Per-row alternatives in report are
retrospective only, not basis for choosing a rule. Additionally all
candidate caps sit on unswept r%4==3 rows, so original construction does
not install these horizontal targets without adding sweep jobs. No
continued implementation or official run recommended.
