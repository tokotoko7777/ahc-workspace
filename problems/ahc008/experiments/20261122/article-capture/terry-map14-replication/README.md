# Fresh scoped map1→4 replication

See [PRESET.md](PRESET.md) for hypothesis, selection and replication gates.
The broader top-two oracle failed; this file does not change that finding.
Only the official generator and prelude-based selection ran during setup.

    python ahc008_terry_map14_replication/prepare.py ahc008_focus_assets/tools/target/release/gen

The fixed uniform stream has 446 eligible cases among 4,096. The first 100
through stream index 900 were locked before any score. The ordered source
seed list and case-record manifest are included. Raw inputs regenerate from
the manifest; no private weights are distributed. The selector uses the
existing locally extracted private include.

Run only in the root's selected serial scoring slot:

    python ahc008_terry_map14_replication/run.py \
      ahc008_focus_solver/problems/ahc008 \
      ahc008_focus_assets/tools/target/release/tester \
      ahc008_terry_motion_cache/solve \
      ahc008_terry_map14_replication/replication.results.csv

The wrapper uses the exact existing top-two board-level official tester,
playback and independent replay checks without altering its source. It
enforces the frozen canonical manifest and interprets only the final
`scoped_replication` object as the gate; the reuse harness's generic
top-two hindsight output is not the gate for this experiment. The
runner up is map 4 on every selected case.

## Independent scoped result: reject

All 200 serial official runs, exact judge playback, independent legality
and score checks passed. Confirmed distinct from the triggering dev
sample by every input hash. No different construction/capture controller
or timeout argument was used.

| Metric | Auto map 1 | Forced runner-up map 4 |
|---|---:|---:|
| Official total | 5,527,744,447 | 5,226,690,276 |
| All isolated | 99 | 83 |
| All and mean floor ≥300 | 99 | 82 |
| Final accessible animals | 2 | 27 |
| Completed t275 simulation pairs | 767 | 2245 |
| Maximum local tester wall time | 2.75135 s | 2.81445 s |

Map 4 won 70 boards, lost 30, no ties. It nevertheless reduced the total by
301,054,171 (−5.44624%) and cut high-area full captures by 17. The raw and
useful fulls as well as total miss the freeze gate. The aggregate decline
and capture loss, not trial frequency or lookback wins, govern rejection.

Descriptive counts, not a basis to narrow or tune the rule:
- M5 (73): 4,105,855,556→3,743,856,943, 48 paired wins, full 72→58,
  useful 72→57; residual 2→25.
- M6 (27): 1,421,888,891→1,482,833,333, 22 wins, full/useful 27→25;
  residual 0→2. Extra points do not justify losing full captures.

No general confirmation, blind-uniform validation, or deployed metric
update. No operator or subgroup changes. The original learned selector
and freeze remain in use, main unchanged. These are conditional generated
inputs, not comprehensive practice or contest judging.
