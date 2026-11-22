# Review of the authorized AHC008 TERRY reference

Reference: [TERRY's article](https://blog.terry-u16.net/entry/ahc008) and
only the Rust text supplied after permission to inspect
[submission #29650242](https://atcoder.jp/contests/ahc008/submissions/29650242).
The text was inspected statically, not copied here or executed.
Original source line references below are to that text.

| Mechanism | Rust reference | Difference in our first integrated pilot |
|---|---|---|
| Legal pen inventory | Five explicitly marked maps, one- and two-mouth rooms, including widths other than four. The five plan masks have 46/62/30/15/54 one-mouth components when all named doors are closed. These are geometric counts, not a dynamic benchmark or claim all are always serviced. | Adapted all rooms to exact two-ended 1×4 to preserve the requested baseline design; this removed a large class of solo jobs. |
| Build order | Build tasks first each turn. In range, hold when temporary blockage prevents an actual wall and keep that FIFO task. A path interval can enter individually from either end, with a length cap. (L635–666, L850–1010, L1100–1163.) | Forecast pair jobs could preempt construction; fully blocked front jobs were skipped. Only the whole snake orientation could reverse. |
| Handoff | Recompute bottleneck assignments every turn over all worker slots, including multiple slots in some lanes, to any reachable row of the lane. (L1181–1365.) | One early assignment to fixed row anchors (once) and role reservation could lag actual human arrival. |
| Forecast and lock | Use retention in the room in which the animal starts the forecast, even allowing return; door squares are counted, and immediate legality is imposed at the real wall placement. Single-ended rooms supported. (L1441–1599, L1646–1751, L1994–2047, L2258–2308.) | Sampled future **joint** trap occupancy plus legality, can include incoming animals, only two-person exact 1×4. |
| Cut and decision | Full-board prize-collecting vertex cut, finite penalties for not enclosing individual reachable pets, route-outs and projected score checks, plus complete-policy continuations in simulations. (L718–818, L2512–2840.) | Capped search over local mandatory cores, greedy approximation of the rail arm, and simulation cuts cannot retry although live can. |

## Frozen development comparisons from the first pilot

Identical 30 development inputs. Each row below is its own paired
intervention, independently verified with official tester, official
replay, independent score, and matching historical baseline hashes.
Used for diagnosis, not to claim a contest result or an adopted solver.

| Change only | Score | Full clear | True four-cell by turn 100 | True four-cell final | Final reachable |
|---|---:|---:|---:|---:|---:|
| Original integrated pilot | 183,714,170 | 0 | 35 | 251 | 184 |
| Individually reversible construction intervals | 136,700,492 | 0 | 30 | 240 | 194 |
| Per-turn nearest-aisle recomputation | 140,809,296 | 0 | 24 | 234 | 201 |
| Construction priority and retain blocked FIFO job | 500,291,219 | 2 | 21 | 315 | 119 |

The first change lowered nominal completion median 84.5→82 but moved
actual first four-cell capture from 84.5→90 among cases with a capture
(30→27); max tester wall time increased 1.10→1.90s. The second change
increased logged lane arrivals (78→112) but delayed first staffing
79→84 and left fewer completed shells. The third raised median ready
shells t80/t100/t150 from 17/22.5/28 to 32/38/48, while still delaying
first capture (84.5→110.5). Preserving a ready workforce is the next
construction-to-capture question, not enlarging the catalog again.

Our reference v7 on these exact cases has 829,740,689 points / 14 full
cases. These results do not establish a new default.

Stacking dynamic lane release **after** construction priority instead is
productive: 500,291,219→727,396,905, whole 2→4, useful whole 2→4,
total exact4 315→340 and final reachable119→93. It still does not improve
initial lock time, 110.5→113.5; the historical v7 is 14 clears. A separate
hybrid retaining the v7 narrower geometry, TERRY runtime front end and
original v7 late fallback also failed the primary adoption check:

| Frozen hybrid vs v7 | Official distributed100 | Independent generated100 |
|---|---:|---:|
| Score | 2,655,369,894 → 2,243,469,252 | 2,688,102,365 → 2,292,445,505 |
| All caught | 36 → 45 | 44 → 36 |
| All caught and ≥300 mean area | 25 → 23 | 26 → 17 |
| Final reachable | 165 → 169 | 139 → 219 |

All 400 replays agreed, and exact reference v7 inputs/actions reproduced.
This hybrid does not replace v7.

Two late-objective / forecast trials from build-first plus dynamic
nearest-lane stage used the same exact board:

| Isolated new change | Paired control score → variant | Full / useful full | Final reachable | True four-cell total |
|---|---:|---:|---:|---:|
| Full-board finite-prize cut | 727,563,572 → 690,969,824 | 4 → 5 (same useful) | 93 → 97 | 340 → 335 |
| Room retention rather than joint future gate-safety | 727,563,572 → 608,136,788 | 4 → 5 (same useful) | 93 → 119 | 340 → 316 |

All official and independent runs were legal. The late Monte Carlo has a CPU
deadline, so a changed count of completed samples can change late decisions
on the same binary and input. Repeated controls missed byte equality in
one/two cases respectively, due to late behavior; do not claim full action
reproducibility for those rows. Their paired controls, not the prior run, are
used for their reported scores. No variant was promoted or tested on historical
judge inputs.

## Reproduction

Use the exact `terry-integration/solve-terry-pilot.cpp` as the initial parent,
and use a distinct filename for every generated source:

- Endpoints: `patch -o endpoint.cpp solve-terry-pilot.cpp < pilot-bidirectional.patch`.
  The independent route unit compiles from `endpoint-unit.cpp` with the archived
  route header.
- Dynamic and construction isolated: `python make-dynamic.py solve-terry-pilot.cpp dynamic.cpp`
  or `python make-construction.py solve-terry-pilot.cpp construction.cpp`.
- Combined: `python make-dynamic.py construction.cpp construction-dynamic.cpp`.
- Soft cut and retention: apply `make-soft-cut.py` or `make-retention.py` to
  `construction-dynamic.cpp`. The folded C++ remains one self-contained file.
- Hybrid: first `python make-narrow.py solve-terry-pilot.cpp narrow.cpp`, then
  `python make-hybrid.py narrow.cpp hybrid.cpp`.

Compile each variant with `g++ -std=c++23 source.cpp -o executable`.
The development harness `bench-development.py` extends the original official
paired runner with physical staging metadata. Reuse the problem's tester,
independent playback, and the archived geometric stage helper (same as the
wide-v11 harness) set up in `PYTHONPATH`. The hybrid uses the original
`terry-integration/bench.py` unchanged. Each compact gzip ledger retains
input/action hashes, score, legal outcome proxy through replay, phase
telemetry and timing for both arms.

### Existing one-mouth true-four openings (no prep walls added)

The five adapted planned boards already include 0/14/18/0/0 one-mouth
straight four-cell pockets. Immediate-only in the 30 development cases
identified 126 legal occupied windows over 8,175 ready pocket-turns, with
zero available humans actually shutting them. These pockets appeared on
4/30 boards. Routing only leftover workers up to four steps after paid
construction and two-person jobs produced 27 dispatches, four true
single-door seals (four heads on two boards). True-four 340→345 and
residual 93→88, but full and useful clear both stayed4. Scores differ only
slightly, 727,563,572→727,724,854, within the scale of a separately observed
late CPU-budget difference on repeated identical controls. Do not claim a
reliable overall score improvement. Use `make-solo.py` on
`construction-dynamic.cpp`, then `make-solo-steer.py` on that result.
`bench-solo.py` records both exact stage and solo closure telemetry.
