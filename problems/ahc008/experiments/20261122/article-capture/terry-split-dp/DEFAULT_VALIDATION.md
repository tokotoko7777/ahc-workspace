# Adopted split DP as usual-clock research reference

[Exact fixed-work validation](RESULT.md) separately verified
−30.15% and −25.93% wait4 CPU across two independent sets of 100,
with frozen scores and all commands. This document concerns only
the different test with the normal 2,670 ms solver budget and no
per-input virtual-pair caps. All 400 actual solver executions match
official tester, official playback and independent replay/score.

| Both default clock | Control → Fast score | Raw / useful full | Total t275 pairs | Max local tester |
|---|---:|---:|---:|---:|
| Official distributed100 | **6,093,833,333 → 6,093,833,333** | 98/98 → 98/98 | 499 → 561 | 2.74947 → 2.74487 s |
| Fresh generated100 | 5,971,694,446 → 5,992,560,186 | 97/97 → 97/97 | 878 → 970 | 2.79529 → 2.81540 s |

On the official distributed set all 100 complete command digests
also match. On 11/100, the fast variant fit more completed pairs.
None fit fewer. On the separately generated set, 15/100 fit more;
none fewer. The latter +20,865,740 (about +0.349%) is from one
case switching Normal→Chase with 38→39 completed pairs. That case
still left animals in both arms. The timed result by itself does not
prove reliably increased scores at a given new case or a reproducible
score change from hardware. No practice score update or rank claim.

Both preregistered default gates pass, so adopt the faster h123
source with unchanged default clock as the reference for research.
No main change. AtCoder distributed numeric target gap remains
169,500,003. Current source SHA-256
`170382b80e4ca1b0d3a5c4f0948527edabfbb43c0549fa8512fbe9dccc59e223`.
The generated source and executable with private user-supplied
selector data are not part of the archive; apply the fixed-work
transform locally and keep the template and validation evidence.
