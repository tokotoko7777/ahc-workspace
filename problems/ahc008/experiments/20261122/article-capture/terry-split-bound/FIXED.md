# Exact native-source startup split pruning, fixed-work results

Construct two feasible evenly divided route assignments. The lower
of their measured bottleneck values is an upper bound; original
state traversal and tie preference remain unchanged. An interval
may be skipped only if its own minimum possible time exceeds the
bound, a monotone stopping condition for the right end. Equal
values remain eligible.

600 original-vs-pruned complete queue/optimum checks match. In
120 paired representative orientations (six workforce counts ×20)
component CPU 6.974009 → 2.502977 s (−64.11%); staff-count groups
−62.88% to −72.41%. Alternating order while timing only the kernel.

| Fixed-work panel (alternated order) | Control CPU | Pruned CPU | Change |
|---|---:|---:|---:|
| Distributed 100 | 57.154542 s | 51.869566 s | −9.25% |
| Separate generated 100 | 63.789951 s | 59.825206 s | −6.22% |
| Distributed no virtual rollout 86 | 20.896761 s | 16.289818 s | −22.05% |
| Separate no virtual rollout 83 | 19.817281 s | 15.623517 s | −21.16% |

All 400 serial official action SHAs and scores match the previously
independently audited original, maximum local tester 2.7491 s.
The fresh rollout stratum alone is +0.52% CPU despite no policy
change, so the measured overall improvement is concentrated before
the terminal rollouts, not an equal endgame speedup claim.
Candidates are built from clean first-Chase memo SHA 8f50dda74f578fc8ddbf8176d15172a80df96d23d82c57db8ae51aedce4294b7.
Candidate SHA f83fd83450294a9730664fa98a76b0dde95c814cd6fd2cce0bcce3635056c088,
binary 5a4a7d9716249e35e78313af50216e0683f86ed71a7af4c0638542e316528c39.
Separate native outcome validation required before adoption.
