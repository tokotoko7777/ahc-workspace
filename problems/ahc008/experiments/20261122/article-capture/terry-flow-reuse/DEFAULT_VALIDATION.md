# Keep score reference: cheaper flow, but default branch choice can change

On fixed workloads, [buffer reuse](FIXED_VALIDATION.md) saves 9.22% /
10.38% of solver CPU among paired positive-rollout boards, while every
complete action stream and score matches. This normal-budget experiment
instead gives both branches the standard 2670 ms without a per-input cap.

| Paired default100 | Score control → buffer reuse | Raw/useful full | Late pairs | Max tester wall |
|---|---:|---:|---:|---:|
| Official distributed | 6,093,833,333 → 6,092,277,778 | 98/98 → 98/98 | 570 → 645 | 2.74532 → 2.73848 s |
| Independent generated | 6,099,433,332 → 6,099,766,666 | 98/98 → 98/98 | 698 → 786 | 2.77384 → 2.76812 s |

All 400 official tester, official playback and independent replays
agree. Final residual is two in all four arms. In each set exactly one
action stream switches; both outcomes on those boards capture all.
Distributed 0055 goes Normal→Chase at 35→42 pairs (−1,555,555),
while independent 0020 goes Chase→Normal at 37→43 (+333,334).
All other 198 pairwise trajectories are identical. The direction
of future score shifts is not guaranteed by throughput. Different
sets are not a historical contest or comparable between-set rank.

Both runtime limits and raw/useful outcomes pass, but distributed
score nondecrease is a preregistered gate. Keep split DP as the score
reference; store the exact fixed-work improvement separately. Do not
change branch choice using these case IDs or retry timing to tune to
known tests. No result promoted or merged into main.
