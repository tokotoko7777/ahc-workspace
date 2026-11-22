# Freeze before equal default-clock comparison

Exactly same independent TERRY method, compare frozen faithful 54f2b1ec
against cumulative faster d14f6471 (ETA-zero miss bypass + static ordered
forecast adjacency). Same absolute t275 budget in both. No workload caps,
case-specific routing, seeds or policy selection. Measure actual score,
full, useful, residual, action/rollout counts and maximum serial tester wall.
Full speed equivalence already separately verified under fixed work; here
any differences may be the t275 time arriving earlier, not a changed plan.

First separately compare original practice100, then fresh uniform100 from
100 deterministic SHA seed derivations under
`ahc008/20261123/terry-cache-default/holdout/0…99`. Manifest frozen before
observing either. Same cases between arms, two datasets not combined.
Require no full/useful regression on either for calling the faster
version the new reference. For score update, require actually improved
reported practice score and no lower fresh score. A timing-driven
single-board increase is a recorded result but not proof of reliably
stronger policy. No main/rank claim. If practice or fresh worse, retain
previous scoring reference; speed result remains exact under fixed work.

## Verified result: select faster implementation, not a new practice best

Both full 100 sets compared at the same absolute 2,670ms cap, sequential,
with all 400 official runs and independent scoring/replay complete. The
fresh panel was frozen by its manifest, not selected for any solver outcome.

| Metric | Practice100 control → cache | Fresh generated100 control → cache |
|---|---:|---:|
| Official total | 6,082,666,664 → 6,082,666,664 | 6,077,388,888 → 6,079,166,666 |
| Whole, with mean floor ≥300 | 98 → 98 | 98 → 98 |
| Total residual | 2 → 2 | 2 → 2 |
| All-turn command SHA equal | 100/100 | 99/100 |
| Number of complete late simulation pairs | 452 → 547 | 723 → 866 |
| Local maximum tester wall time | 2.7346 → 2.7305 s | 2.8130 → 2.8326 s |

Every activated row (14/21 respectively) increased pairs. On fresh 0081,
46→55 pairs changed the decision from chase to normal, being the only
score change (+1,777,778). Exact command-hash reproduction of both arms
confirmed first divergence at action t275. This is a single timing-window
effect, not reliable evidence of a higher-quality underlying search.
The code speed is confirmed separately with identical work. We select the
cache solver for continued research, without changing the practice high
score 6,082,666,664 or claiming a historical contest ranking. No main merge.
