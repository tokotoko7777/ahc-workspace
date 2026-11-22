# Integrated article-inspired AHC008 pilot

Independent implementation, local assembly from immutable global-clear standalone
`591d608…` (blob `9448476d8cf3571bc0519a1ec431add302ef7165`). The two user-approved
narrative references are [TERRY](https://blog.terry-u16.net/entry/ahc008) for staged
construction, actual arrival-time occupancy, global disjoint pair selection,
idle-worker targeting and late escape-aware min-cut; and
[siman](https://simanman.hatenablog.com/entry/2022/02/28/074838) for species-aware
four-cell traversable tubes, reduced low-M walls, directional cues, and the
importance of reaching the last pet. No other entrant material or source code used.

`python assemble.py && g++ -std=c++23 integrated.cpp -o integrated`; real pragma
in base. No runtime arguments. Unit: `g++ -std=c++23 test.cpp -o unit && ./unit`.

1. Use verified two-bank (M≤6) and three-bank (M≥7) layout with brace-first
   construction. Actual ready row counts allow hunting after t35 and five
   prepared tubes in any bank; cap at one tentative pair until two-thirds of
   tubes ready, except release that cap at t101. All unclaimed workers keep
   constructing at most through t132. This experiment combines stages but
   does not replace inherited brace construction with article route scripts.
2. Pair-ready, exact legal, occupied four-cell two-end closures are selected
   independently first, maximizing current animals isolated for distinct
   workers and tubes. Later forecast plans are nonbinding and cannot replace
   these. In forecast assignment, worker BFS avoids each other prepared
   tube's four body cells; a human starting in a tube can leave it.
3. Forecast passive on the real graph to actual joint ETA up to ten turns
   (cow 1, pig 2, rabbit 3 uniform substeps). Dog posterior is conditioned on
   actually observed directed transitions and actual post-human graph; its
   first three forecast turns propagate hidden target/reset under frozen human
   positions, followed by two uniform substeps per turn. Cat approximates
   only first-step heading (50% recent observed heading, 50% uniform;
   subsequent uniform); no latent cat target or altered human path considered.
   Predicted count uses the no-gate-block probability across pets via
   independence. Compare arrival-time expectation discounted by ETA and retain
   a weak prior-plan tie preference. The weighted hyperedges are packed
   exactly with a per-tube worker-mask DP; correlations between multiple
   different candidate tubes are not jointly modeled.
4. After building, unmatched workers are assigned distinct pet targets by
   weighted bitmask matching, steering near staffed stances; remaining
   unassigned workers use distributed prepared stance standby.
5. From t180 if only one or two observed animals remain human-reachable,
   an independent compact-pocket planner has first opportunity. Preserve actual
   root-connected service aisles and exterior crossbars in candidate cut search
   when possible, or retry with protected retreat anchoring at the crew root.
   Either requires all humans reach retained connected territory and an
   occupied detached component, retains minimum area, and checks safety at
   each action. Failed/stalled detachment returns control. Ordinary baseline
   fallback starts at t235 if last-pet planner not owning the turn; no t160
   fixed handoff. Existing no-unsafe-human repair and all-clear stopping remain.
6. `AHC008_ARTICLE_LOG` columns zero-based action turn, mode (0 construct,
   1 partial, 2 hunt, 3 final protected roads, 4 ordinary fallback, 5 final
   crew retreat), total prepared tubes, pets in chosen simultaneous closures.
   True capture/size/shape measured by out-of-process official replay, not
   by this diagnostic count.

Freeze SHA256 `a9c6f8c17bf02728c3f29de1e675ab928b72bfbdbc50b2ee30986dd276cc0e7e`.
Without geometric pruning, weighted set packing scans at most
`36 × 2^10 × C(10,2) = 1,658,880` mask-option checks per turn,
once per stage (ordinary forecast and rarer immediate), after collapsing
orientation-equivalent worker masks. Most masks are unreachable and most
worker pairs cannot arrive at opposing stances in ten exterior steps;
actual cost measured by the parent-controlled replay. Exact ties preserve
the first strict improvement in deterministic tube, worker and mask order.
This means stationary broad ties are not symmetry-randomized. Critically,
pair routes/classes are recomputed each turn; we use a small prior bias,
not an irreversible reservation. The weighted set objective can benefit
from disjoint assignments but does not give a probabilistic all-capture
guarantee, and candidate formation excludes ETA>10 altogether; idle
repositioning is the bridge to distant service lanes. If elapsed CPU
already exceeds inherited 1.65 s, no new forecast or min-cut is started,
but legal immediate captures remain available.

## Dev30 validation, not promoted

`gzip -dc integrated.cpp.gz > integrated.cpp` restores the exact standalone.
For source rebuilding with `assemble.py`, supply the immutable global-clear
standalone above as `base.cpp`. Both compilation paths yield the same source.
The archived official replay command for the old 30 dev cases is
`python bench_article.py HARNESS TESTER GLOBAL_CLEAR INTEGRATED DEV30 report.csv`;
add this repository experiment directory to `PYTHONPATH` for replay helpers.

All 60 scores matched official recorded replay and independent legality and
score verification. Global-clear→integrated: official local score
746,886,882→535,150,247, all-clear 18→5, final distinct residual42→104,
confirmed exactly-four capture/terminal pens212→286, nonrectangular
4–48 animal isolation53→23, sterile human area union sum5885→2384.
Residual at t100/160/225/275: 353/239/161/111→321/203/155/130.
Median/.max wall seconds .301/.411→.703/.883. No broader run or promotion
of this exact first composition. The paired hunter clearly increases small
early captures but the new handoff sees too many residuals for the 1–2
specialist, and ordinary late cleanup removes 51 animals from t225 versus
119 in the control. This comparison changes the whole integration and
cannot independently estimate causality by stage or by individual feature.
