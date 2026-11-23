# One fewer room: static 14-slot witness fails weak-color traffic gate

Independent full Map3 overlay passes: 74 exact rooms including the
same 60 other rooms, 233 planned X, all open floor connected, no
orphan under all locks, and all original outside roles remain
exterior under every individual lock. The left bank has seven
both-color rooms at the original 44 X wall budget, replacing 15
old roles with 14. The model uses one passive animal, complete
construction, no other blockers, and instantaneous staffing.

| 25-turn narrow access | Stock Map3 | Fixed 14-slot bank |
|---|---:|---:|
| Pig weaker-color 10th percentile | 14.02% | 33.96% |
| Pig weaker-color mean | 58.17% | 60.68% |
| Pig overall mean | 63.61% | 68.18% |
| Cow overall mean | 70.18% | 70.33% |
| Rabbit overall mean | 86.52% | 87.18% |

The color means (69.07%, 58.17%) change to (60.68%, 76.11%).
The fixed weak-color mean gate is 65%; improvement of aggregate
mean alone is not enough. Reject without a dry/live test or layout
retuning. No runtime, construction, capture-rate, or score claim.
Reproduce from workspace using `python3 ahc008_terry_map3_width3_14/traffic.py`
with the prior atlas, cap and packing scripts. Same-domain
independent probability implementation matches both full
per-position vectors within 1e-14.
