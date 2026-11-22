# Strict proactive empty-aisle bridge scheduling

One v4 variant; no cage layout or capture-criteria changes. Only after all
configured shoulder walls are ready and t101–152, after actual legal
two-door captures and the ordinary selected prospective pairs/brace jobs,
an otherwise unclaimed person at an exact existing exterior mouth
may traverse a different open and unoccupied prepared four-cell tube
in seven moves. Another person must occupy the donor lane and destination
lane must currently have no occupant and another open prepared side
stance within three cells. Any bridge already chosen for an ordinary
prospective pair is also forbidden. One active traversal, reserve its
person/tube and two gate cells (no capture or new predictive job in that
tube) until exit; no override of any already-paid current job. The
latest possible exit is t159, before original t160 handoff, and the
person immediately resumes ordinary generalized scheduling. Defensive
route interruption retreats where possible; no default phase changes.

Restore v4 standalone and apply `proactive-bridge.patch`. Source SHA256
`65ef9c66a898ccd564d3874de5bcc422962ffe444da2511d057bf9a73ada364b`.
Build no-args `g++ -std=c++23 integrated.cpp -o integrated`.
`g++ -std=c++23 bridge-unit.cpp -o unit && ./unit` beside candidate
standalone validates strict no-op preconditions (paid holding, another
person on destination, occupied pen, unavailable wall, already
paired bridge, early/late cap, incomplete braces), guarded traversal
and safe retreat during artificial path interruption.

Matched used dev30 with official/replay legality all60:9 active
cases,14 attempted and14 completed traversals (98 moves, zero stalls
or retreats). Unchanged prefix t0–100 on all30 and full bytes in all21
inactive cases. v4→v7 all-clear 13→14 (3 gained,2 lost), residual45→49;
real4 236 unchanged, irregular4–48 63 unchanged, score866,421,245→
829,740,689 (5W/21T/4L), normalized area ceiling −2.14%; sampled max
.873→.885s. This weak win in all-head count does not justify replacing
v4 by itself; opposite residue and area direction and only five
discordant cases. No tuning based on individual IDs.

## Matched frozen official distributed practice100, unchanged variant

All100 exact v4 archived official action hashes and both t0–100 prefixes
matched; all200 official legal/recorded playback scores are exact.
21 active cases, 24 starts and completed crossings (168 moves), 79
inactive complete byte identity. v4→v7 all-clear33→36 (5 gained,2 lost,
31 both,62 neither); residual166→165; actual exact4 814→822; irregular
4–48 207→204; score2,598,735,635→2,655,369,894; retained-area ceiling
−0.68%; max wall .986→.946s (runtime sample, no performance claim).
Substantive whole-head gains retained 521,436,648 mean clean cells,
largest initial isolation components 94,127,16 respectively.
The other gained cases retained179 and36; 36-cell case is refuge-like
and first isolated five pets into a single 630-cell room. Lost clear
cases retained168 and624 in v4. Counts therefore don't imply uniformly
compact fully cleared cases. Exact residual distribution, per-case pen
areas at first separation and gained/lost people-weighted area are in
the paired report. Check via `paired_analysis.py V7_LEDGER V4_LEDGER`;
concatenate decompressed A+B pieces (B headerless).

## Frozen independent official-generator 100, unchanged variant

Same independent generated100 frozen before v7, with official v3 seeds
below and exact v4 controls. All100 previous v4 hashes match, both arms
exact t0–100 prefix throughout,79 no-start cases completely identical.
21 active cases and27/27 completed bridge crossings (189 moves).
All200 official/independent playback legal. v4→v7 all-clear41→44
(7 gains/4 losses/37 both/52 neither), final residual152→139, true
exact4 830→837, real irregular4–48 218→221, official score
2,519,824,436→2,688,102,365, normalized area ceiling −0.10%;
max .934→.870s (sample only). Newly clear mean floors
666,510,621,138,445,158,192; lost v4 clear floors500,171,2,383.
The 666-cell first case had all animals in exactly-four pens; 138-cell
case included a 441-cell first-isolation component. Practice100 full-clear
with ≥100 floor33→35 (excluding v7's 36-cell refuge); independent38→42
(excluding two v7 and three v4 small cases). Full-clear with ≥300 floor
practice22→25, independent24→26. Analytical area bins don't redefine
official raw all-clear.

`gzip -dc integrated.cpp.gz > integrated.cpp` is exact frozen standalone,
`g++ -std=c++23 integrated.cpp -o integrated`, no args. Independent
seeds use the official v3 `gen`; retain order as archived. V7 provides
small measured full-isolation gains in both cohorts with area largely
preserved; large residual shortfall remains. These are local evaluation
sets, not hidden system judging or first place.
