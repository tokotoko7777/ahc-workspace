# Predict arriving animals: adopted with 100/100 distributed full clears

Original h123 forecasts only animals already in numbered rooms. The
variant adds exact h1–3 endpoint probability for animals that currently
start outside those labels but within the actual majority-human region.
Cows, pigs, rabbits and cats (still the stock two-step symmetric
approximation) are included; dogs still use the existing logic. Next
endpoint must be inside a valid room and personally allow both named
gate placements. Existing same-room values, RNG stream, room shapes,
staffing, tie order, legal output and native clock are otherwise
unchanged. Multiple room endpoints are collected from one bounded
integer probability propagation per considered animal.

The preregistered detached screen passes: same observed command
prefixes on all 60, 2,515 changed detached first proposals, none
loses an immediate stock isolated ID on the same board, and 38
different ETA opportunities on 27 boards with all other work/static
movement retained. This is only feasibility under observed paths;
it does not execute an alternative, predict causality or score it.

| Actual paired default runs (each 100) | Split reference | Incoming | Score gain | Raw full | Useful full | Final remaining |
|---|---:|---:|---:|---:|---:|---:|
| New generated development | 5,990,111,109 | 6,055,944,442 | +1.099% | 95→98 | 95→98 | 6→2 |
| Precommitted untouched generated | 5,949,695,369 | 6,051,812,965 | +1.716% | 94→99 | 94→98 | 9→2 |
| Official distributed practice | 6,093,833,333 | **6,152,805,559** | +0.9677% | **98→100** | **98→100** | **2→0** |

All 600 actual runs match the official tester, official playback
and an independent replay scorer. Maximum tester wall after the
change: 2.77125, 2.75314, 2.74771 s (respectively). The official
set has 44 case wins, 16 ties and 40 losses; all action streams
change, with 11 terminal mode differences. The two practice baseline
misses (both cats) now isolate on actions 274 and 279, state275 and
state280 respectively; this reflects the policy as a whole, not
proof either capture was a direct single forecast effect. Among
98 shared complete cases total per-case mean human area +43.25
cells; score +4,805,560. Prior dev/hold gains mainly come from raw
full changes. Case IDs/timing in the CSV are diagnostics only and
are not supplied to solver decisions.

Promotion gate passes on each set; use as the new research reference.
Official distributed numerical gap to 6,263,333,336 is 110,527,777
(~9.95 mean human cells per board under full capture). These are not
historical contest inputs and no historical rank or main merge is
claimed.

## Reproduction

From the independent accepted split C++23 source, use the included
make.py (detached observer), make_live.py and the exact arrival
histogram unit fixture. The pure fixture was matched independently
to 360 independent random path-tree enumerations plus deterministic cases. Source
SHA-256
`6c59f0145870fe050f0528938ebd037342791073f6fa04c4870a01e34cbc6b40`.
Build passes plain `g++ -std=gnu++23`, compiler optimization in source.
Use the same paired official harness as the split default, selecting
the archived fresh100 seed lists and input SHA manifests, or the
official's distributed cases for practice. Measure the same serial
2670ms budget; per-case time-driven simulation counts may vary.
No original source, supplied selector weights, generated solver
carrying them or binary is redistributed. Archive includes only
independent transforms, pure tests and benchmark evidence.
