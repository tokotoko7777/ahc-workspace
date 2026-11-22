# AHC008 v25-fast2: an isolated wider forecast test

One change on [pinned exact v25-fast2](../v25-fast2/README.md): 64 → 128
counter-based guard trajectories on five-worker boards only. First 64
trial draws identical at the same state; geometry, early eligibility,
other actions, and global/late deadlines unchanged.

On fresh unused M5 development pilot30, all60 official legal and
independent replay. Score 405,947,424 → 420,701,324, raw full 11 → 6,
useful≥300 2 → 1, final residual 78 → 101. Actual four-cell isolation
by state 100: 73 → 75; by160: 197 → 197; final204 → 203.
Worst local test .611 s. Reject under predeclared rules, no test of
held stream. More samples do not guarantee a better decision.

Regenerate the first 30 cases from `panel/` per preset; run the same
paired evaluator. Source builder and exact ledger are in
`evidence.tar.gz`. No change to selected v25-fast2.
