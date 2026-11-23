# Map 3: prebuild eight mouths to repair the poor pig color

The 25-turn first-passage atlas found a strong pig-color asymmetry
in Map 3. A stock 3-cell paired room can capture only from its midpoint
that is not adjacent to either gate, which a two-step pig of the other
color will never occupy. Prebuild one of its two own gates. The room
becomes a one-mouth 3-cell room with two gate-safe cells (one of each
color). Keep all outside stances and all other room masks unchanged.
Eight places were selected without any animal cases or scores, using the
predeclared greedy objective on the same exact completed-board graph.

| One passive animal with assumed staffing | Before | Eight commissions |
|---|---:|---:|
| Pig 25-turn narrow-hit, worse color 10th percentile | 14.02% | 37.92% |
| Pig worse-color mean | 58.17% | 65.64% |
| Pig overall mean | 63.61% | 67.40% |
| Cow overall mean | 70.18% | 69.80% |
| Rabbit overall mean | 86.52% | 86.96% |
| Planned X (Map 2 reference 260) | 233 | 241 |
| Exact named room bodies | 75 | 75 |

Gate IDs (zero-based flattened): 215, 335, 455, 155, 575, 695, 824, 815.
At every candidate, the all-open floor remains connected and each
one-room closure leaves that exact body and one connected exterior.
Every original worker stance remains in the exterior, including
the stances used for the new planned caps. All 56 one-cap choices
were valid at launch. The full selected plan passes the frozen
static gate, with the final weak-color mean threshold exceeded
by about 0.64 percentage points. No clock, judge score, capture
rate or pet behavior during construction has been measured here.
The next distinct gate is dry construction and staffing at equal
initial starts. Map 3 was not substituted for the more reliable
Map 2 when estimating or measuring any score.

From the workspace, `python3 ahc008_terry_map3_pig_parity_caps/caps.py`
uses the earlier atlas and exact stock terrain/census. The recurrence
produces the same full baseline vector as the atlas's independent
Python implementation within 1e-14 at all available cells.
