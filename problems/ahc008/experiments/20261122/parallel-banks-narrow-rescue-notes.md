# Reduced scaffold for five/six humans (development)

Use 2×13 banks, 112 brace targets, three lanes and both crossbars for M<=6;
the no-argument rescue 3×12 is still the research default. Visible flags
`5 1 1` enable this low-worker experiment. 26 tubes extend through row 27
(zero-based); lateral offsets 4, 8, 12 and both orientations use the old
visible placement objective. First staffing at five ready tubes within a
bank, global second phase at 18 of 26 (versus 24 of 36), t101 stable jobs.
With five humans duplicate a pair of neighboring lanes (choose either pair
by the same minimum sum of stance distances); with six duplicate all lanes.
Exact simultaneous closure and post-bank tactical are unchanged, including
the release trigger >4 reachable pets at t160, otherwise 225. No M>=7
behavior change, no route/ranking experiments in this branch.

Used the first 30 M<=6 official-generated cases of a fresh independent
140-seed file, selected by human count only, and eight first M>=7 as
identity controls. This is a screened *development* set, not confirmation.

| M<=6 dev30 | 3-bank rescue | 2-bank rescue |
|---|---:|---:|
| Official score | 430412475 | 473614991 |
| Actual atomic exact-four bank captures | 174 | 186 |
| Final exact-four animals | 180 | 193 |
| t160 residual | 262 | 250 |
| Released at t160 | 27 | 25 |
| Final residual | 100 | 89 |
| Full clear | 4 | 8 |
| Distinct clean area sum | 1477 | 2430 |
| Cleared people | 30 | 50 |
| Ready at t50, pooled | 427/1080 (39.5%) | 417/780 (53.5%) |
| Ready at t101, pooled | 800/1080 (74.1%) | 702/780 (90.0%) |
| Braces t50, pooled | 2699/4680 | 2405/3360 |
| Median first true capture from t0 | 69.5 | 67 |
| Median/max tester seconds | .373/.621 | .295/.464 |

Score W/T/L 16/0/14. Direct species counts cow/pig/rabbit/dog/cat:
[20,25,16,86,27]→[18,30,17,84,37]. Earlier progress is relative to a
smaller station inventory, not more absolute ready tubes. Scores, legal
actions, genuine exact-four captures and always-connected bank service were
checked by official v3 plus exact playback. All 76 trajectories pass;
both arms match full geometry and exact action bytes in all eight M>=7
controls. No adoption based on screened dev30.

Reproduce using the official generator on the archived seed list from
the intended cases directory (it writes under `in`). Pick first 30
sorted inputs whose visible M is <=6 and first 8 whose M is >=7 from the
same stream; symlink into `$NARROW/eligible` and `$NARROW/identity`.
Benchmark harness must provide benchmark.py and benchmark_cases.py.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-narrow.cpp "$D/parallel-banks-rescue-research.cpp" "$D/parallel-banks-narrow-rescue.patch"
g++ -std=c++23 "$D/parallel-banks-rescue-research.cpp" -o /tmp/ahc008-rescue
g++ -std=c++23 /tmp/ahc008-narrow.cpp -o /tmp/ahc008-narrow
python "$D/parallel_banks_narrow_rescue_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-rescue /tmp/ahc008-narrow "$NARROW/eligible" /tmp/narrow-dev.csv
python "$D/parallel_banks_narrow_rescue_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-rescue /tmp/ahc008-narrow "$NARROW/identity" /tmp/narrow-id.csv --high-workers --case-count 8
```

## Frozen independent100 and official distributed practice100

The source/options did not change after dev. Exact experimental executable
SHA256 `bd39755108dd0063c1c78422bebcd4bfa5188b708413de6f12a1fedc5c837dee`.
The generated holdout was unselected from a new independent stream;
practice is the official v3 distributed100, not competition judge input.

| Metric | Independent100 old → narrow | Practice100 old → narrow |
|---|---:|---:|
| Exact official score | 1811107220 → 1916659829 (+5.83%) | 1953389032 → 2116997192 (+8.38%) |
| M<=6 eligible-only score | 195506745 → 301059354 (+54.0%) | 375962426 → 539570586 (+43.5%) |
| Score W/T/L | 16/75/9 | 23/66/11 |
| Direct simultaneous exact-four animals | 730 → 746 | 688 → 692 |
| Final exact-four-pen animals | 751 → 760 | 700 → 706 |
| t160 reachable animals | 807 → 793 | 815 → 809 |
| Final residual animals | 192 → 208 | 230 → 220 |
| Fully clear cases | 32 → 34 | 31 → 30 |
| Distinct clean union | 10464 → 11417 | 11156 → 11820 |
| Cleared people | 286 → 297 | 279 → 286 |
| Max wall seconds | .695 → .676 | .616 → .616 |

Exact same geometry and full action bytes for every unaffected M>=7 case:
75/75 fresh and 66/66 practice. All 400 official runs legal, same exact
scores under recorded action playback and independent replay, confirmed
no strand in all bank-stage turns and all counted direct captures have
exact four passable cells. All 100 old practice arm transcript hashes and
scores match the earlier freeze. Each individual paired descriptive
bootstrap interval spans zero; both cohorts nevertheless show better score,
more exact-four captures and more clean union. Both final residual and
all-clear are mixed across cohorts, so this is not a reliability-dominant
replacement. No main or contest submission changes.

For each mixed100 use the same commands as dev but pass
`--mixed --case-count 100`. Use the archived holdout seed file with the
official generator, and the extracted untouched `tools/in` in the
other run. Exact per-case gzip ledgers include legality implicit in the
replay assertions, timings, individual action hashes and capture counts.
JSON ledgers reproduce via `parallel_banks_narrow_rescue_summary.py`.
