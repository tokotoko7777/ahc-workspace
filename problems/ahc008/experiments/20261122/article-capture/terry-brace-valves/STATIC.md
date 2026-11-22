# Deferred proximal brace valves: geometry research only

Input is the accepted faithful static terrain catalog, maps 2/3/4. No solver or baseline changes. No seed, action or outcome enters geometry selection. Eligibility is an original straight, same-row paired-door room of width at least three with both guard stances on true aisle markers. Pair exact equal-span bodies two rows apart, greedily earliest first and disjoint in the ordered bank. The valve is only the proximal (leftmost interior column) cell on their common separator; require the right separator neighbor still a wall and a physically non-wall, non-gate staff stance immediately left. One additional step left must reach a configured aisle. No alternate orientation or spacing search. Map1 not included.

Before compilation handoff:
```sh
g++ -std=c++23 -Wall -Wextra -pedantic ahc008_terry_brace_valves/feasibility.cpp -o /tmp/terry-valve-fixture
/tmp/terry-valve-fixture > ahc008_terry_brace_valves/catalog.txt
```

`generate` yields original/initial terrain, candidate valve cells, original named gate identities and stock-room IDs. It does not issue commands or activate a phenotype. Static test checks every original named undeferred body/door footprint, all candidates opened simultaneously, isolated recovery of either nominated pair after its own restoration while all others remain open, no phantom numeric rooms, and service aisle paths under original single gate actions.

Policy guard for any future use: omission is a preparation choice, not a new live seal. Mark *both* attached stock rooms unready while the valve is open, even if all other stock braces are present; continue stock approach/traffic prediction but no ordinary closure for these rooms until the valve is legally built and physically certified. Valve is an independent controlled one-wall commitment from its exterior stance. Reserve stock routes/crew and forbid same-turn movement versus the placed cell. With restored valve, both original gates are unchanged; prior existing walls and pet placements still require replay-safe flood connectivity, legality and reachable guard stance checks. Cap on restoration effort is a future policy question, not part of this static test.

Preselected geometry kill: weighted count of candidate skipped X is below 10/board on the incoming frozen cohort. Later diagnostic on verified tapes only: old attached-room factual seals and exact jointly-safe final-state omission (fictional reopened walls, no score forecast). Investigate only if the mean retained floor slack is at least 6 across the whole cohort; actual causal acceptance still requires higher aggregate and no useful or whole capture loss. Do not use practice to change pairs.

## Frozen results (no solver execution)

Unmodified predetermined rules pass exact static verification: 21 candidate valves on map2 (42 attached rooms; 33 untouched), 28 on map3 (56 attached; 19 untouched), zero on map4 (21 eligible two-mouth rooms, but no adjacent eligible equal-span rows). No candidate assigned to maps0/1. The atlas is `catalog.txt`, including both associated original room IDs/door pairs and the real outside stance.

```sh
python ahc008_terry_brace_valves/held_bound.py > ahc008_terry_brace_valves/held_report.json
```

In the incoming held100 archive (all hashes checked, frozen stock 99 full), mix 18 map1, 36 map2, 36 map3, 10 map4. The menu covers 17.64 installed X per board, with 10.98 never needed for an exact factual attached-room first isolation. Exact simultaneous **fictional** wall omissions yield 10.88 additional scored human floor per board (map2 +12.3611, map3 +17.8611, positive on all 72 eligible boards); all safe gains are from candidates whose attached rooms had no factual stock isolation. The joint bound opens every valve not adjacent to stock pet floor: forbidden adjacent valves can never be on a safely reachable human path, while simultaneously opening the remaining ones cannot connect to a pet component. Each stock human stays at their original endpoint. The gain is diagnostic only; leaving these holes open in actual time affects early construction, guard availability, mouth traffic and late capture. Both preregistered static screens pass; this provides no live adoption evidence.
