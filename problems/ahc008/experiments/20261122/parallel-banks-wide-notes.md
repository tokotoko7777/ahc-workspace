# Cover more transverse width with a fourth staged bank (M≥7 only)

Freeze the earlier staged controller; add a bank only when the input's
number of humans is at least seven. The 4×12 body-only brace geometry
uses 208 side cells and five connected aisles with column 0 or 1 offset,
chosen by the same visible position metric and two orientations.
Existing 3×12 uses 156 braces and four aisles. Every bank's individual
staffing trigger stays at five ready tubes and no earlier than t35;
full widening threshold is 2/3 ready (24/36→32/48). Retain t101 full
staff assignment, t133 setup work stop and t225 unchanged tactical
fallback. A fourth bank uses the same target priorities over an
independent set of non-adjacent lane pairs; no new forecast. Geometry
may change orientation/site because the same ranking metric is
applied to the wider candidate catalogue.

Fresh enriched dev30: generate the 100 archived SHA-derived seeds
then take the first 30 inputs with visible M≥7, in original order and
without consulting outcomes. Compare reference staged 3×12 and new
M-gated policy. Also run first 8 from same generator with M<7 and
check absolutely identical command bytes and geometry. All 76 outputs
official/legal and exact replay. Every human and all service lanes are
connected throughout bank stage through t225; credited closures are
simultaneous in the official trace and exactly 4 connected cells.

| Metric, 30 high-worker cases | Staged 3×12 | Staged 4×12 |
|---|---:|---:|
| Direct compact animals | 215 | 256 |
| Seal pairs | 212 | 254 |
| Final actual exact-four animals | 219 | 265 |
| Cow/pig/rabbit/dog/cat directly banked | 35/32/29/63/56 | 51/52/38/67/48 |
| Per-50-turn direct captures (last block t200–224) | 9/65/68/46/27 | 4/68/81/77/26 |
| Distinct eligible case-pets | 406 | 447 |
| Eligible uncaptured case-pets | 191 | 191 |
| Legal ready but unstaffed motif-turns | 2377 | 3063 |
| Mean ready at t101 | 35.07 / 36 | 40.73 / 48 |
| Mean ready at t225 | 36.00 / 36 | 45.27 / 48 |
| Mean braces at t101 | 153.33 | 187.23 |
| Mean braces at t225 | 156.00 | 201.13 |
| Residual at t225 → final | 255 → 214 | 214 → 178 |
| Human union mean at t225 → final | 701.6 → 608.3 | 648.1 → 610.1 |
| All-clear cases | 3 | 0 |
| Official score | 227,961,076 | 177,130,499 |
| Maximum measured tester wall seconds | .515 | .751 |

Paired direct capture wins/ties/losses 21/3/6, normal score 14/0/16.
The fourth bank increases genuine compact coverage and lessens final
residual animals; it adds 41 eligible distinct case-pets and captures
41 additional overall (possible identities differ). This does not
produce fully clear cases or score gains. More raw legal windows are
unstaffed. Treat as a separate exploratory compact variant; no default,
holdout, distributed or contest changes.

Reproduce with official v3 and the seed file (first-eight SHA digest
bytes little endian of `ahc008/banks/wide4-enriched-dev-v1/{i}`, i0..99).
Apply parallel-banks-pilot.patch, parallel-banks-staged.patch then
parallel-banks-wide.patch to a scratch tactical source, GNU C++23.
CLI3=staged baseline, CLI4=wide-when-eligible. No other archive result
informed case membership.

    python parallel_banks_wide_bench.py "$HARNESS" "$TESTER" ./tactical ./banks-wide "$M7PLUS" wide.csv
    python parallel_banks_wide_bench.py "$HARNESS" "$TESTER" ./tactical ./banks-wide "$MUNDER7" low.csv --low-workers --case-count 8
