Read-only M5 regular post-construction cap2 union overlap audit on exact pinned v25-fast2.
    python ahc008_v25_union_audit/make.py ahc008_v25_fast_static/solve.cpp ahc008_v25_union_audit/solve.cpp
    g++ -std=c++23 ahc008_v25_union_audit/solve.cpp -o ahc008_v25_union_audit/solve
Run actual original policy with unique AHC008_UNION_AUDIT=RAW/CASE.log through root runner. Confirm historical actions/input hashes and legal/replayed scores (instrumentation can alter time budget; mismatching boards unverified).
    python ahc008_v25_union_audit/summarize.py RAW/*.log

Uses precisely original sampled paths and offered deduplicated jobs. Among feasible two-pair plans with distinct users/tubes and preserved service connectivity, preserve original additive per-hit weights and compare exact expectation of each intercepted ID credited once at the greater applicable weight. Subtract only shared-trial/ID intersections times lesser weight. Same-ETA disjoint body intersections are zero. Preserve original plan on ties, not pure unweighted union or independence. No switch to one pair and no call in cap1. If original two-pair plan fails simultaneous static connectivity guard, log and skip, do not falsely attribute its replacement to overlap. Alternatives use unchanged per-tube/mask job prefilter and do not search jobs previously discarded. Half checks descriptive only, no resampling. Candidate movement differs only in forecast first commands, and original paired commands must survive postprocessing without replacing a later-issued nonwait on the only possible newly recruited worker for the main count. No alternative executed. This ignores trajectories' physical reactions to different human positions/walls. Five distinct boards with real first-command reversals required before considering live implementation.

## Verified M5 development comparison

All 30 inputs/actions and official scores matched known v25-fast2,
independent replay also checked. This was read-only. In 1,822 actual
two-pair decision turns, 1,722 had differing-ETA alternative pairs.
99 states offered positive duplicate joint credit somewhere; the
incumbent selected pair overlapped in 19 states (27 sampled
pet×trial occurrences). Correcting by exact same-trial union reverses
eight assignments, but only three boards and three turns have actual
noncollateral first-move changes. All three point the same way in
the descriptive split halves (not independent performance evidence).
Preregistered five-board gate not met. No live, held or rank claim.

The pinned generator, run scripts, summary and raw data are in
`evidence.tar.gz`. Selected [v25-fast2](../v25-fast2/README.md) unchanged.
