# Event-keyed virtual pair streams: reject

All 22 signed t275 prefixes and both historical stock sums for each of
their 1,184 reproduced fixed-work pairs matched. The fixed sample has
10 distributed and 12 independent cases, selected by what record was
available under the prior Map1 and late-large archive rules. 20 current
native Chase and 2 current Native Normal choices; not representative.
Each detached comparison then ran 16 pairs per stream on a clean state.
Synthetic unbiased bounded draws, event/retry isolation, and equal-state
trajectories passed.

| Measure | Result |
|---|---:|
| Median keyed/stock paired difference variance ratio (lower better) | 0.910926 |
| Cases with lower keyed variance | 16 / 22 |
| Mean normalized paired variance, stock → keyed | 0.02716479 → 0.02776423 (+2.21%) |
| Mean normalized paired variance, independence control | 0.02912390 |
| Serial stock → keyed CPU | 3.955085 → 4.085912 seconds (+3.31%) |

The fixed primary gate was a median ratio at most 0.90, which was not
reached, and pooled mean variance also went in the wrong direction.
Arm-mean drift checks and the CPU ceiling passed. Do not change trial
seed, retry protocol, or threshold. No alternate policy was issued, no
official test, no current solver change. This detached result concerns
estimated variance of the virtual policy pair only.

See PRESET, FROZEN, and OBSERVED for local reproduction and the full
result digest. Repository records include the independent observer and
sampler code; the private assembled benchmark contestant and binary
are not included.
