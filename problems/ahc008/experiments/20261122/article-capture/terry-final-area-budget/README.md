# Factual whole-capture area budget

Run from the workspace root:
```
python ahc008_terry_final_area_budget/audit.py
```
Reads the accepted exact-short-passive h123 fresh60 report and whole
tapes from the equal-flow cut audit. Independently matches all three
hashes and replay scores; no solver/tester runs. Uses the existing
private faithful map selector to identify planned cells, not to score.

For a full board let U be the union of all human components. The final
scored mean area M is each person's own component size averaged across
people, so the exact identity is:
`900−M = planned X + extra walls + nonhuman pet floors + empty floors + (U−M)`.
No double counting walls, pets, empty regions or human splitting.

59 full of 60, one unfilled case 0059. Across 59 full boards:

| Term | Total cells | Mean per case |
|---|---:|---:|
| Built planned X | 14,971 | 253.746 |
| Extra walls | 1,560 | 26.441 |
| Pet-only component floor | 3,611 | 61.203 |
| Empty component floor | 5 | 0.085 |
| Human splitting penalty | 0 | 0.000 |
| Scored human floor | 32,953 | 558.525 |

The only empty components are 4 cells in 0004 (five boundary walls;
last human-reachable state 280) and one in 0021 (four boundary walls;
last human-reachable state 290). Both boundaries complete at the listed
action number, so isolation first holds following that action. On the
frozen final board, exact 0–1 shortest path permits reopening one wall
for 0004 while preserving no human/pet connectivity. That is a planned
X from action80, not the late closure. In 0021 there is no safe
reopening path even allowing many wall removals: any path must run
through a pet component or an opened wall adjacent to it. These are
fictional final-state wall removals, not legal operations or evidence
for skipping wall construction or closing differently in real play.
Empty space and human split contribute essentially no direct headroom.
