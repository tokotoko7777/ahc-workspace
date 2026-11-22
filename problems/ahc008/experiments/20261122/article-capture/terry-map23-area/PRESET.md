# Frozen feature-only 2→3 ablation, faithful baseline unchanged

Hypothesis: original map 3 (233 planned X, five longitudinal aisles, mixed one/two-mouth sites) may recover area versus original map 2 (260 X, four aisles) when stock NN considers them similar and the starting crew can cover the extra aisle. The 27 planned wall saving is only direct geometry, not a forecast of net territory/capture. Room patterns differ as in the original, and no controllers, capture simulations, times or walls are modified.

Eligibility is fixed before any outcomes:
1. The input has 7 or 8 people and the exact stock selector chooses map 2.
2. Its raw sigmoid scores have 0 ≤ score2 − score3 ≤ 0.04. This is a fixed four-point indifference band on the model's 0–1 output, chosen to ask a near-tie question; not calibrated win probability, not fitted to previous panels.
3. Compute one distinct real person per longitudinal aisle using exact minimax BFS on that original map's final planned-X-only graph (hypothetic guard travel, initial true positions, all gates still open; omit animals/build costs). Require map3 finite and its bottleneck no more than 2 movements slower than map2. Two moves are a deliberately small reservation for one extra aisle, not a tuned coefficient. Report actual completed construction and guard handoff time separately: this screen does not guarantee them.

Each case is generated from deterministic SHA-256 first 64 little-endian bits of namespace ahc008-terry-map23-neartie-v1:{dev|fresh-confirm}:i for 0≤i<4096. Seed files are committed in both folders. Screening reads only the public prelude (species counts, people count, original starting positions); never future movements or scores. First 30 dev eligible by stream order, not by margin or expected result; if fewer, do not silently enlarge or relax. Untouched confirmation panel targets first 100 strictly eligible from other namespace key, held until candidate decision. No previous official distributed or generated held score data used. Bench if explicitly approved: same solver both arms, stock auto selection vs original --map=3 only on the eligible cases. To isolate version, benchmark only one nominated faithful implementation (frozen or accepted speed-identical derivative). Outcomes: exact legal replay, full, useful≥300, per-head human area, score, worst runtime, credible completion and handoff. Candidate worth fresh verification only if dev total score and mean area of paired all-clears rise and full/useful do not fall; no promotion until the same directional result on independent cases.

    python screen.py dev --seeds-only
    # Once root releases the timing slot (this is generation/feature audit, no tester):
    python screen.py dev --generator ../ahc008_focus_assets/tools/target/release/gen

Current state: seed commitment and script prepared, eligibility count pending permitted generation. Never run official timer with this script.

## Fixed development result: rejected

Eligibility flow: 4,096 seeds → 1,336 with M7/8 → 886 auto-selected
map 2 → 264 NN gap ≤.04 → 185 passed the distinct-worker reach screen.
The first 30 qualifying inputs were fixed without examining outcomes.
Run the identical frozen faithful source 54f2b1ec twice, with no flags
(auto map2, CSV arm `v25` is the generic harness name and **not** the
older policy) versus only `--map=3` (CSV arm `terry_independent`).
All 60 serial official runs, official playback, and independent score
checks succeeded.

| Metric | Original map2 decision | More-open map3 |
|---|---:|---:|
| Score | 1,886,444,446 | 1,740,319,445 |
| All caught | 30/30 | 24/30 |
| All caught and average area ≥300 | 30/30 | 24/30 |
| Final residual | 0 | 8 |
| Mean human area among same 24 all-clear pairs | 565.33 | 589.96 |
| Median nominal construction estimate | 72.5 | 67.5 |
| Median first forecast assignment | 79 | 75 |
| Max local tester wall time | 0.632 s | 2.818 s |

All 24 jointly full matches gained score; each of the six lost full matches
lowered score, so aggregate score is worse. Additional finishing space did
not compensate for lost capture, even though the first nominal milestones
were earlier. The held stream remains ungenerated/unused, no selector
change, no further threshold tuning on these cases. The original five-map
faithful baseline stays in use.

Replay command (standalone source name and binary as in faithful archive):

    python ahc008_terry_exact_root/bench.py ahc008_focus_solver/problems/ahc008 ahc008_focus_assets/tools/target/release/tester ahc008_terry_exact_root/standalone ahc008_terry_exact_root/standalone ahc008_terry_map23_selector/dev/selected recheck.csv --candidate-arg=--map=3
