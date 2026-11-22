# Adopted: exact feasible upper-bound startup pruning

Serial GNU C++23 native internal 2670ms, official judge, official
playback and independent replay. All 400 runs checked. All 200 A/B
full action hashes and scores identical; no late-mode switches.

| Inputs | Both scores | Both full/useful | Simulated pairs | Max tester wall |
|---|---:|---:|---:|---:|
| Distributed 100 | 6,152,805,559 | 100 / 100 | 667 → 673 | 2.7624 → 2.7437 s |
| Separately generated 100 | 5,998,857,137 | 97 / 97 | 942 → 944 | 2.7606 → 2.7589 s |

Same-time pair counts differ naturally; no extra score is claimed.
See FIXED.md for isolated same-work CPU reductions of 9.25% / 6.22%
on the entire solver, concentrated in the setup component. Scores are
not equivalent to historical placement. Goal 6,263,333,336, observed
numeric gap 110,527,777, no submission or main merge.

Source SHA256 f83fd83450294a9730664fa98a76b0dde95c814cd6fd2cce0bcce3635056c088
and binary 5a4a7d9716249e35e78313af50216e0683f86ed71a7af4c0638542e316528c39.
Reproduce `bash ahc008_terry_split_bound/run_default.sh` after
`python ahc008_terry_split_bound/make.py` and `run.sh` checked in fixed archive.
Only the deterministic transformation and evidence may be shared;
assembled solver with supplied private model is not redistributed.
