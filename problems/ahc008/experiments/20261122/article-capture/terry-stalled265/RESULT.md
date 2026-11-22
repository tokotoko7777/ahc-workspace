# Stalled at t265: forced earlier Chase rejected
First 100 qualifying boards after 610 uniform seed slots (max 2,048), selected from factual 265-action prefix only. Both complete arms passed all 200 official tester, official playback, independent scores, signed input/prefix checks and separate replay eligibility checks. Unopened reserved stream, no deploy or further subgroup selection.

| Result | Native f83 | Forced actual-turn Chase265 |
|---|---:|---:|
| Score | 5,278,344,442 | 5,264,793,386 |
| Raw full | 79 | 89 |
| Useful full | 78 | 83 |
| Total residual | 23 | 14 |
| Local tester max seconds | 2.855762 | 0.417780 |

Delta −13,551,056 (−0.25673%); 33 score wins, 59 losses, 8 ties, 100 changed action streams. Board-resampled one-sided 95% lower bound −198,776,983. Five individually full→miss; preset score, bound and full preservation failed. Forced runtime omits the stock rollout decision, so not evidence of deployment headroom.

| Final status | Boards | Exact paired score delta |
|---|---:|---:|
| Both full | 74 | −135,188,891 |
| New full | 15 | +229,348,152 |
| Lost full | 5 | −108,133,333 |
| Both miss | 6 | +423,016 |

Since full cases have no pet discount, the both-full score loss corresponds to about 16.44 fewer reachable floor tiles per human on average per board, via the official linear formula; this is approximate from rounded scores, not a direct area ledger. Verified playback traces were temporary. Individual final floor and worker split metrics are not persisted, so no exact decomposition into these causes is possible without new replay, and none was run. These are conditional experimental results, not an unconditional or historical rank comparison.
