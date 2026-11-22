# Top-two hindsight diagnostic: scripts and locked 100-case panel

See [PRESET.md](PRESET.md) for comparison, no-outcome-selection protocol,
2.5% advancement gate and ungenerated held namespace commitment.
Only the official generator and prelude evaluation have been run in setup.
The prepared development inputs are local; they are regenerated from the
included seed list. Private reference coefficients and assembled solvers are
not part of a shareable archive. `bench.py` refuses to use an alternate
solver executable or input that does not match the original dev manifest.
The full replay harness and selector remain their existing verified code.

Reproduce the prelude/manifest from workspace root (the selector's existing
private include is expected and is produced by the prior exact-selector
importer):

    python ahc008_terry_top2_oracle/prepare.py ahc008_focus_assets/tools/target/release/gen

Run only as an explicitly selected serial job:

    python ahc008_terry_top2_oracle/bench.py \
      ahc008_focus_solver/problems/ahc008 \
      ahc008_focus_assets/tools/target/release/tester \
      ahc008_terry_motion_cache/solve \
      ahc008_terry_top2_oracle/dev \
      ahc008_terry_top2_oracle/dev.results.csv

Use a separate destination for attempts; on failure inspect the last
complete pair and error from the official tool, and do not omit that case
or reuse incomplete aggregates. The summary computes the score-best oracle,
tie to original. Paired alternative use of seed outcomes is hindsight by
definition; it is neither a predictor nor an original contest result.

## Verified dev100, broad gate fails

All 200 serial official executions, exact judge playback and independent
score calculations passed. Each board is uniform, from the new frozen
100-input manifest. All controls choose exact stock NN top1 in program;
all forced arms used only the same NN's runner-up for that prelude.

| Measure | Stock top1 | Forced runner-up | Hindsight higher score |
|---|---:|---:|---:|
| Total | 6,147,833,331 | 5,930,666,669 | 6,271,444,445 |
| Whole clears / useful ≥300 | 98 / 98 | 97 / 97 | 100 / 100 |
| Final accessible pets | 2 | 3 | 0 |

Runner-up won 25/100, no ties. Oracle gain 123,611,114 (+2.01065%);
55,388,890 from the two rescued stock misses, 68,222,224 from 23
both-clear improvements. This is derived from outcomes and cannot be
used as a live score, ranking comparison, or evidence to train after the
predeclared ≥2.5% broad gate failed. The previously planned general held
panel was neither generated nor used; no broad predictor built.
Local max tester wall 2.7498 s stock, 2.7729 s runner-up.

A separate descriptive subgroup, stock map1 and runner-up map4, improved
all 11 dev cases (+31,222,224 total), an observed hypothesis not a
deployment rule. Test only this distinct hypothesis in a new precommitted
100 qualifying inputs, with independent acceptance and no further rule
changes; then, only if confirmed, whole fresh uniform validation. It does
not override the failed broad gate and cannot, by itself, close the whole
practice score gap. Old selected result/reference unchanged.
