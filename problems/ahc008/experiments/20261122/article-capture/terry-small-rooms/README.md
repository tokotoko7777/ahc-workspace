# Earlier small exact-room opportunity, factual screen only

Build catalog fixture (not the solver) and run tape screen from workspace:
```
g++ -std=gnu++23 -O2 ahc008_terry_small_rooms/catalog.cpp -o /tmp/terry-room-catalog
/tmp/terry-room-catalog > ahc008_terry_small_rooms/catalog.tsv
python ahc008_terry_small_rooms/audit.py > ahc008_terry_small_rooms/report.json
```

The catalog gets actual original roles and single/double mouths from
faithful terrain classification, not an arbitrary separator. Small means
physical body of 1–4 cells (excluding the gate cells, which would be
walls). For each real Normal snapshot, require the precise physical room
to be fully cordoned by built shoulders and named doors; each added door
is open and legally buildable by distinct current, aligned actors. Preserve
every existing wall and every unrelated walking command. The projected
physical component must match the body both before and after replaying
the non-replaced commands, and no existing exact stock pet isolation may
be lost. Classify actors' overwritten dots separately from any redirected
walk. A dot may include a paid hold, so dot-only is still an optimistic
proxy for fully unreserved, rather than asserting invisible job freedom.

Compare only when all actual occupants of a later-caught larger room are
present in the earlier small room. Confirm the same occupancy roster at
original capture and terminal. Deduplicate by later terminal compartment
and keep its best geometric floor difference (not score, no gate credits).
These witnesses do not certify that saved terminals would be accessible,
or subsequent movements would be unchanged.

All100 exact action/input digests and independent replay scores checked.
95 boards have 1,348 occupied exact small room-turns with legal gates
but before staffing; just six valid aligned alternatives on five boards,
all requiring movement replacements, produce 13 summed compartment
floor cells. No dot-only cases, no stock wall or actual other capture
displaced in the six. Both predeclared gates (20 boards, 200 cells) fail.
No official tool, live suggestion, or threshold widening.
