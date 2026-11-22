# AHC008 TERRY-faithful terminal rollout clock correction (one-factor)

Frozen reference preserves a literal choice of the original submission:
its virtual chase planner is invoked with argument 275 on all 25 rollout
turns; the live chase controller uses actual turn. In particular, moving
through the virtual late steps never reduces the reach radius or relaxes
the final-turn routing restriction. This experiment changes only the virtual
chase call to receive actual simulated turn. Keep switch at the actual 275,
normal-policy branch and RNG, review criteria, physical geometry, movement,
chase objective and live chase fixed. No earlier switch, threshold, cutoff
or prediction retune. No benchmark score was used to choose this correction.

Stage 1: diagnostic on the two existing 100 suites (practice first). Feed
both arms the same actual confirmed t275 completed-pair count per board to
isolate the internal clock parameter. This offline cap is never a live
parameter or instance routing and does not expose seed IDs to the solver.
Validate with the official tester and independent official playback and
score replay. Require baseline exact frozen score/action hashes, candidate
pre275 prefix equal, no actions changed on any no-pair board, score higher
and no lower full/useful on both if advancing. On fixed-work candidate pair
aborts record and don't omit or pretend the workloads matched. If practice
fails the gate, do not run independent or tune here.

Stage 2 only after both diagnostics pass: freeze source, benchmark default
clock once on fresh uniform generated100 from an unobserved new manifest,
plus the same previously reported practice100 to measure numerical target;
no use of the previous per-case pair caps in live. Require useful and
raw full not down, score up, legal all, meaningful timing reserve. No
historical rank claim and no main merge merely from local gain.

## Comparison protocol follow-up, no solver changes

Fixed-work mode was stopped and rejected as workload-equivalent after the
candidate completed 18 rather than 19 pairs on practice0015 at the same
2,670ms absolute wall cap. First 15 complete pairs and the exception remain
archived; do not drop the slow board or present a same-work improvement.

To answer the separate deployed-performance question run both *unchanged*
solvers at default 2,670ms on all practice100 (equal available time, different
number of pairs allowed; use the freshly measured control, not the old
fixed-cap score). This is a time-limited selection comparison, not a
mechanism-independent speed comparison. Advancement requires strictly
higher score, no fall in raw or useful fulls, equal prefixes in all 100,
and all legal with time reserve. This is a method change only, done before
any full pilot result. If it fails, do not run the independent panel.

## Verified equal availability result: reject

All 200 serial official outputs and official/independent replay checks
passed. The one-action-prefix check is complete: 99 full command SHA
matches, and sole changed case 0055 reproduced with the exact candidate
score/action SHA and its observed 25 pairs, then first differing action
was confirmed at zero-based index 275; all 275 earlier actions identical.

| Metric | Frozen faithful control | Actual virtual turn |
|---|---:|---:|
| Score | 6,082,666,664 | 6,081,111,109 |
| Whole clears | 98/100 | 98/100 |
| Whole with mean human area ≥300 | 98/100 | 98/100 |
| Final reachable animals | 2 | 2 |
| Maximum local tester wall time | 2.731 s | 2.741 s |

Case 0055 switches from normal to chase, old/new completed pairs 27/25,
and score 63,111,111→61,555,556; exact score loss 1,555,555. The default
scorer is timing-dependent (12 input counts changed; total443→447),
so this shows the live equal-time behavior on this run, not a fixed-count
comparison. No independent test, no adoption or repeat tuning.

Candidate private standalone source SHA:
`7a9f6617b201824fcdd4d2c7d55fc8b372c221b6e4b3834fac449337878e0bff`.
Archive includes the generator (one-line edit), capped workload harness and
partial result/explicit abort, default result and exact prefix proof. The
assembled original-data-carrying source or binary is not in this archive.
