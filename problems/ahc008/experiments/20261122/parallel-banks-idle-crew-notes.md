# Borrow idle secondary crew after bank handoff: no action opportunities

Follow-up to rejected one-crew dispatch, audit only. Apply diagnostic patch
to exact 591d608… global-clear source (blob 9448476d8cf3571bc0519a1ec431add302ef7165),
compile via `g++ -std=c++23 global-clear.cpp -o idle-crew`, then run
`parallel_banks_idle_crew_audit.py HARNESS TESTER DEV30 idle-crew
global-clear-dev30.csv events.jsonl summary.json`.
All 30 original global-clear action hashes and official/independent playback
scores reproduced exactly.

Log only actual ordinary dual invocations where after both planners the
primary has a target and the secondary has none. Test actual secondary
people who still wait after all ordinary legal repairs or gate handling,
placing only currently pending walls directly adjacent to external
(non-shell, non-body) stances. Compare against every actual ordinary move
and wall, require exact legality, distinct builders and no splits in any
ordinary postaction human component. Planned final-missed exposure counts
only if the complete projected primary shell truly isolates that current
animal with all human groups intact; no assumed future animal behavior.

Only eight applicable turns in seven cases. None even reached the
adjacent exterior legal stance check, so no real supplemental build,
detachment or final-missed exposure. This does not test moving or lending
secondary workers when a second plan does exist. No live arm.
