# Reject: fixed count-preserving one-mouth geometry on new native100

After independent closed-board geometry and 35/35 build/safety dry
checks, A/B uses official fixed seed-namespace dev100 at native
2670 ms, paired serial. All 200 runs verified with official tester,
official playback and independent legality/score. The reserve and
official distributed candidate tests were not run. All 23 unchanged
maps reproduce exact commands and scores in this pilot.

| Cohort | Score base → candidate | Full/useful base → candidate |
|---|---:|---:|
| All100 | 6,049,960,322 → 5,994,050,593 | 95 → 94 |
| Map2 (33) | 1,984,611,111 → 1,959,388,888 | 31 → 31 |
| Map3 (44) | 2,843,555,559 → 2,812,868,053 | 43 → 42 |

Three full clear adds vs four drops. One extra residual head overall.
Common fully clear edited70 mean floor **−4.1857**, so failure is
not explained just by capture count. All edited cases build their full
nominal X eventually: +154 in sum. Actual total unweighted mean-worker
area −483.375 tiles and added pet penalty +19.81256 equivalent;
these sum to −55,909,729 aggregate points within rounding.
The edited77 have +28/+60/+94/+39/+24 reachable heads at t75/100/160/
225/275; on common clears70 +30/+59/+85/+32/+21. Thus lost early
throughput persists even in successfully cleared boards, requiring many
more late trials (816→1774 all100). Max tester wall 2.7599→2.8262 s.
This does not attribute the difference to any one route or layout cell.

Reject the frozen live gate, no tuning or reserve runs. The adopted
original selected solver remains 6,152,805,559/full100 on official
distributed100, not historically comparable to the provided target.
Candidate SHA256 dcb5f9ca63e2cba0afa95ac654a31342279f1d243bb1659d75f81e9ed12ff7b5
and binary SHA 0ade7bd03a9329d3553e5ef31cc593b10d6b87e60aa05544ea607cf8e2d8ee98.
See LIVE_GATE.md and evidence; no original-private model in archive.
