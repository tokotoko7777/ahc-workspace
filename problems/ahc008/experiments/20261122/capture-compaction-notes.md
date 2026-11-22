# AHC008 capture room compaction

Keep the tactical score leader and capture-first baseline separate. All development arms below ran on the exact same 30 fresh official-generator inputs as the first capture experiment, with official test and independent action replay. Pet-only area is the union of distinct components containing pets and no humans, not area per pet or an assumed pen volume. An arm is not a compaction improvement merely because pet-only area decreases while more animals stay in the human area.

| Variant | All-clear / 30 | Mean pet-free person's area | Score |
|---|---:|---:|---:|
| Capture4 baseline | 30 | 94.88 | 297,777,775 |
| Partial occupied-leaf harvesting | 28 | 80.42 | 263,444,442 |
| Partial harvesting + wait for larger union | 25 | 109.66 | 316,523,001 |
| Wait for larger clean union, no pruning | 23 | 117.23 | 323,506,285 |
| Uneven density geometry | 29 | 81.03 | 274,333,331 |
| Uneven + pruning | 22 | 86.36 | 233,852,049 |
| Instant guarded fallback enlargement | 30 | 94.88 | 297,777,775 |
| Synchronous full-room exclusion, ≥2 pets | 27 | 89.38 | 265,916,202 |
| Synchronous full-room exclusion, ≥1 pet | 21 | 124.91 | 308,603,952 |
| Local corner exclusion, ≤4 prep turns | 30 | 87.23 | 270,444,442 |
| Cow-led local corner, protected fallback | 30 | 95.98 | 301,111,109 |
| All-species local corner, protected fallback | 30 | 83.96 | 265,888,885 |
| **Immediate-only local/whole-room closure** | **30** | **116.57** | **367,333,332** |

All timed runs legal. See corresponding development CSV and JSON for timings, and topology sidecar CSV for group measurements.

## Paired all-clear comparisons

On cases where both solvers exclude every pet, compare like with like:

| Candidate | Pairs | Distinct safe area: base → candidate | Total pet-only area | Largest pet-only component |
|---|---:|---:|---:|---:|
| Whole-room ≥2 pets | 27 | 91.96 → 88.59 | 655.89 → 658.15 | 638.19 → 601.19 |
| Whole-room ≥1 pet | 21 | 102.86 → 126.67 | 644.48 → 615.52 | 620.00 → 426.14 |
| Local corner, 4-turn prep | 30 | 89.33 → 81.13 | 658.53 → 666.57 | 641.40 → 656.37 |
| Cow-only protected fallback | 30 | 89.33 → 90.33 | 658.53 → 657.43 | 641.40 → 640.30 |
| Immediate-only local/whole room | 30 | 89.33 → 110.20 | 658.53 → 637.50 | 641.40 → 610.70 |

All pairs here have one connected human component. Preparatory travel often hurts escape or diverts workers from a larger clean union. The promising arm spends exactly one simultaneous construction turn when the workers are *already* adjacent; no partial closes and no new walking to stage a pen. It checks exact remaining connectivity, counts removed pets and limits area loss (28 for a corner, 90 for an entire macro tile). This is evaluated separately from the fallback-guarded enlargement, which found no opportunity in this pilot.

## Reproduction

The immediate-only frozen source has all options compiled in: g++ -std=c++23 problems/ahc008/capture_compact_submission.cpp -o compact. The capture-first baseline is compiled similarly. Benchmark with official v3 tester + generator and --capture-topology topology.csv; that option emits an extra untimed metric sidecar with all occupied components, distinct residual/isolated pets, pet-only area, and mixed area. Use paired_compaction.py SCORE.csv TOPOLOGY.csv capture4 compact to summarize both-all-clear pairs.

The neighborhood patches are diffs against capture_first_submission.cpp and may be applied to scratch copies. Benchmarked compile-time arms:
- room_harvest: CAP_GRID=4, CAP_DOG=0, CAP_READY=150, CAP_START=150. Harvest: CAP_HARVEST=1. Harvest+wait: CAP_HARVEST=2 and CAP_FLOOR=190. Wait alone: CAP_HARVEST=0 and CAP_FLOOR=220. Uneven: CAP_LAYOUT=1 and CAP_RET=18. Uneven+harvest: CAP_LAYOUT=2 and CAP_HARVEST=1.
- capture_expand: CAP_QUICK_EXPAND=1 or 2.
- capture_synchronous: default synchronous minimum is 2; minimum 1 used CAP_PEN_MIN=1 with CAP_PEN_END=238.
- capture_local_cows: corner with prep: CAP_PEN_MIN=1, CAP_PEN_END=178, CAP_PEN_ETA=4. Protected cow-only: CAP_PEN_START=45, CAP_PEN_MIN=1, CAP_PEN_END=162, CAP_PEN_ETA=4, CAP_PEN_PROTECT=1, CAP_PEN_COW=1. Protected all species: substitute CAP_PEN_END=165 and CAP_PEN_COW=0.

The distributed baseline topology data reproduced exact prior scores and action hashes. In its 98 all-clear cases the existing capture4's animal-only commons were still large (mean 664.97 cells; mean largest 658.28), so the first milestone establishes separation, not small individual animal cages.

## Frozen immediate-only verdict: not promoted

The candidate performed well on the pilot, then was frozen unchanged (including the standalone source) before the following unseen official-generator100 and separately distributed100. New generator seeds: first eight SHA-256 bytes little-endian of ahc008/20261122/capture-compaction/immediate100/{i}, i=0–99. The distributed set was not used to choose any option.

| Dataset | Capture4 all-clear | Immediate all-clear | Capture4 score | Immediate score | Immediate worst tester wall |
|---|---:|---:|---:|---:|---:|
| New independent generated100 | 98/100 | 99/100 | 915,277,777 | 884,722,220 | 0.351399 s |
| Official distributed100 | 98/100 | 97/100 | 912,125,586 | 901,324,507 | 0.356260 s |

All 600 candidate/control/tactical executions across both datasets are legal, and independent replay matches official tester score. On the paired all-clear subsets:

| Dataset | Pairs | Distinct human-safe area | Distinct pet-only area | Largest pet-only component |
|---|---:|---:|---:|---:|
| Generated | 98 | 83.72 → 80.64 | 664.96 → 667.92 | 659.70 → 656.49 |
| Distributed | 97 | 84.12 → 83.18 | 664.63 → 665.47 | 657.87 → 657.13 |

Both paired subsets retained a mean of one human component, i.e. one group in every paired case. The candidate did not compact total occupied area or enlarge the clean region reliably. Keep capture_first_submission.cpp as the capture baseline; the tactical solver remains the score baseline. Neither should be described as achieving the original score goal.

Next structural hypothesis: execute the same selected safe region with concurrent guard positioning while the rest evacuates (matching all guards to reachable inside door stances before closing, no gate closes while an unassigned human is still outside). This changes stage ordering, not which initial clean subset is selected. It removes delay without sacrificing fallback preparation to separate hunting. For actual small pens, repeat short observed-occupancy corner closures without redirection only when enough guards appear near the same retained backbone; long preparations empirically lose opportunities.

## Concurrent refuge gate staffing: promising but not adopted

Keep the same initial connected clean subset and later closure/legality checks. While workers evacuate, choose a bounded matching of distinct door guards to reachable inside positions, timed to arrive within 0–2 turns of the usual slowest evacuee. Existing closure waits for all humans to be inside, so no unassigned person is stranded by the first closing move.

On dev30, 0-slack staffing 27/30 all-clear, 384,112,455 score; 2-slack staffing 28/30, 453,889,598, versus 30/30 and 297,777,775 baseline. On the 28 paired all-clear cases of 2-slack, safe union 92.46→145.89, pet-only area 655.43→602.32, largest pet-only component 637.07→591.79; one human component in every paired case. First durable sampled all-clear (every 10 turns) mean 167.14→160.36, median 160→140. Max tester wall 0.248660 s, no illegal outputs. The speed of sealing is suggestive, but the decline from 30/30 to 28/30 violates the capture-first priority. The protective follow-up changes positions only of builders already inside the selected region, and refuses a next step leaving it.

Raw per-case pilot outputs are grouped in capture-compaction-pilots.tar.gz; list with tar -tzf and extract with tar -xzf. The two frozen immediate-only validations remain as directly viewable CSV/JSON and topology sidecars.

### Frozen inside-only staffing verdict: not promoted

Development30: 29/30 all-clear against 30/30 control. The losing case 0005 ends with all five people and all 17 pets still mixed in one 747-cell component, rather than a newly contaminated small refuge. A guard-staging decision can affect animal paths and later target choice even when initial outsider moves are nominally unchanged; this case was not asserted to diagnose causality.

Independent generated100 seed string is ahc008/20261122/capture-compaction/inside-staff100/{i}, converted with the same SHA rule. The unchanged candidate is archived as capture_staff_candidate.cpp. Both generated100 and separately distributed100 use the official tester and independent action replay, with no illegal runs:

| Dataset | Baseline → staff all-clear | Paired reliability wins/losses | Baseline → staff score | Staff worst wall |
|---|---:|---:|---:|---:|
| New generated100 | 100/100 → 98/100 | 0 / 2 | 826,444,442 → 832,001,590 | 0.246110 s |
| Official distributed100 | 98/100 → 97/100 | 0 / 1 | 912,125,586 → 872,257,068 | 0.276809 s |

| Dataset | Paired all-clear | Safe union | Pet-only area | Largest pet-only component | Durable clear mean sampled turn |
|---|---:|---:|---:|---:|---:|
| New generated | 98 | 74.90→76.41 | 673.89→672.40 | 672.45→670.46 | 160.31→161.02 |
| Official distributed | 97 | 83.61→80.93 | 665.12→667.89 | 658.36→663.59 | 160.52→165.36 |

Median first durable sample is 150 for each arm in both datasets (10-turn samples), with one connected human group in every paired-clear case. The initial measurable staffing speedup is not reproduced. Stop staffing coefficient tuning.

## Next experiment: guarded observed local entrances

Instrument animal entries into 2–4-cell-deep corners alongside existing macro walls. Before modifying the solver, measure *conditional* chance of another step taking the observed animal at least two cells from the remaining gate, stratified by species, distance, and guard movement cost. Next, reserve only workers already on the fallback side and within one move of that last gate. Close after observed entry if exact board legality and full team connectivity hold, and do nothing if the entrance does not become immediately ready. Unlike waiting for a static empty tunnel from arbitrary starting positions, this conditions on animal approach and reuses completed macro walls. Do not spend new motions on preparing a larger pen, and switch immediately back to an already selected clean refuge when it is still viable. Require unchanged all-clear rate plus reduced total pet-only area on same-clear pairs, not merely a smaller largest pet component.
