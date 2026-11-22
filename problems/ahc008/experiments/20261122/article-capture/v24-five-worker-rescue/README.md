# AHC008: five-worker layout and route-preserving rescue

This is a research checkpoint, not an adopted update. The baseline remains
[our v24 C++23 solver and original evaluation](../v24-source-release/README.md).
Full means no animal in any human-accessible component at state 300. Useful
full adds a mean human-accessible area of at least 300 cells. All comparisons
below use the same 30 generated five-worker development cases, not AtCoder
system tests; legal outputs and scores were checked with the official judge
and an independent playback. Baseline total on this set: 434,537,087.

## Findings

- Exact baseline trajectory audit: in seven of eight full-but-narrow cases, one
  legacy turn abruptly takes the group below 300 cells, losing 217–732 cells.
- All six single-parameter v24 layouts (2 orientations × offsets 4/8/12):
  180 validated runs. The actual baseline-selected layout reproduces all 30
  exact action hashes. Eleven of 180 individual runs were useful full,
  spanning eight cases. Retrospectively choosing a highest-score run per case
  would yield 857,769,441, 18 full and 8 useful; any layout can yield full in 25
  cases. This describes only these six choices and uses outcomes unavailable
  during play. It is not a solver or a result for Terry's Rust original.
- An operational opening estimator moved effective staff-pair readiness from
  median turn 77 to 70, but full clears declined from 10 to 8 and useful full
  remained 2. Intervening only for an observed legal double closure during
  setup, without dropping scheduled wall tasks, never activated.
- A bounded single articulation cut, using only real board edges, discards
  at most 30% of the present component and keeps every worker connected
  with at least 300 cells. It chooses from legal immediate closures and safe
  exterior-only walks of at most four turns during the midgame rescue.
  Across three ablations, none increased useful full clears:

| Rescue policy | Score | Full | Useful full | Pets left |
|---|---:|---:|---:|---:|
| v24 | 434,537,087 | 10 | 2 | 62 |
| Replace one turn with a cut job | 459,717,357 | 11 | 1 | 85 |
| Only immediate single cuts | 433,034,023 | 8 | 1 | 101 |
| Preserve compatible legacy actions; approach only with an idle worker | 474,868,467 | 9 | 2 | 75 |

The higher score of the last row did not solve the all-capture problem and
was not promoted. Next compare jointly planned irregular two-gate closures
which leave workers connected, rather than unconditionally preferring the
first individually useful cut.

## Reproduce

Pinned v24 source must have SHA-256
`7ae8fc4c25a5b69c448682b62c428fb73f83b7a2d826003f07b2ea173be06639`.
For the layout audit run `python generate.py --v24 solve-v24.cpp --output layout.cpp`,
then compile `g++ -std=gnu++23 layout.cpp -o layout` and use
`--m5-layout=0` through `--m5-layout=5` as separate passes.
The forced flag is for diagnosis, not part of a deployment policy.

For the rescue ablations `python make.py solve-v24.cpp spine.cpp`, then
`g++ -std=gnu++23 spine.cpp -o spine` (first row after v24),
`g++ -std=gnu++23 -DAHC008_SPINE_IMMEDIATE spine.cpp -o immediate`, or
`g++ -std=gnu++23 -DAHC008_SPINE_MERGE spine.cpp -o merge`. Optimization is
in the source. Generate inputs with the official tool from `manifest.json`
in manifest order and verify the recorded input digests. Use the supplied
`bench.py HARNESS OFFICIAL_TESTER V24 CANDIDATE CASES RESULTS.csv --n 30` with
the repository's `problems/ahc008` harness. Add the AHC008
`experiments/20261122` and `article-wide-v11` directories to `PYTHONPATH`
for independent trajectory and construction statistics.
`run_labels.py` and `audit.py` are the original workspace scripts and
retain their original workspace paths; their outputs are archived here.
No comparison with Terry's original or first-place status is claimed.
