# Staff active banks as soon as actual braces are ready

This comparison starts from the original body-only bank pilot (not
the rejected short-horizon joint scheduler). Same 3×12 geometry, all
wall jobs, direct closure, target ranking, t101 ordinary full assignment
and t225 tactical fallback. Starting no earlier than t35, a first bank
with ≥5 physically ready tubes gets two lane guards. Others still build.
At ≥24 ready total, expand only to neighbor lanes of another bank with
≥5 ready. One thread per lane, no new density/forecast changes. At
t101 reassign full staff just as prior from current true positions.

Old dev30, all 60 official/legal/action replays. Verified 30/30 original
bank action hashes for the reference, identical initial geometry and
t0–34 bytes across arms, and the actual service aisles + all humans
connected through t225 in both arms, independently of which tubes close.

| Metric | Prior hard t101 | Staged |
|---|---:|---:|
| Exact-four direct animals | 133 | 219 |
| Exact same-turn two-gate seals | 127 | 210 |
| Captures by 50-turn blocks [0,49], [50,99], [100,149], [150,199], [200,224] | 0/0/40/55/38 | 10/68/69/49/23 |
| Cow/pig/rabbit/dog/cat | 21/24/17/46/25 | 23/37/38/77/44 |
| Median first seal among capturing cases | 133.5 | 58 |
| Average tubes ready at t50 | 21.63 | 20.20 |
| Average tubes ready at t101 | 34.90 | 32.33 |
| Average tubes ready at t225 | 35.63 | 33.67 |
| Average actual brace cells at t101 | 153.47 | 145.23 |
| Average actual brace cells at t225 | 155.27 | 149.27 |
| Eligible unique case-pets | 378 | 380 |
| Eligible but uncaptured case-pets | 245 | 161 |
| Legal ready-but-unstaffed motif windows | 2711 | 2020 |
| Reachable pets, t225 → final | 302 → 265 | 216 → 174 |
| Mean human area at t225 | 719.3 | 708.7 |
| Mean final human union (unique cells) | 644.2 | 608.1 |
| All-clear cases | 1 | 2 |
| Official score | 69,288,715 | 232,815,017 |
| Max wall tester seconds | .557 | .610 |

Capture paired wins/ties/losses 21/4/5, score22/0/8. This is an 86
animal (+65%) compact improvement on the used development cases, not an
independent check. Early staffed cases capture many during the original
inactive 100-turn interval and still catch more in the next block; the
small brace delay is measurable but does not eat the gain on these cases.
Compared with untouched tactical 603,294,259 and 165 final residual pets,
score remains much lower: this does not remove enough residual animals
often enough. Treat as separate stronger bank candidate, no adoption.

Compile prior tactical, then in scratch apply parallel-banks-pilot.patch
and parallel-banks-staged.patch, in GNU C++23. CLI1 is prior, CLI3 is
early staged. Bench:

    python parallel_banks_staged_bench.py "$HARNESS" "$TESTER" ./tactical ./banks-staged "$CASES" compare.csv
