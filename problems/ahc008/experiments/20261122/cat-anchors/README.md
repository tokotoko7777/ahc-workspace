# AHC008: generate new min-cut cage centers from cat motion

Local isolated add-on to `mincut-speed.cpp` at `26911ec` (the control is the standalone `control.cpp` bundled from that checkpoint). All tests used newly generated seeds, not official distributed cases or contest final input. No remote edits.

## Heuristic and invariants

Each cat maintains an EMA of its **directed first move** (.47 new, .53 previous). Net displacement is the difference between opposite direction weights. For a confident cat in a human-accessible component, BFS over the actual current walls to depth six, keeping the best accessible center at graph-distance 2, 4, or 6, above a directional projection threshold. Ranking of endpoints penalizes lateral displacement and nearest-worker travel; max nine extra centers per selection. Candidate centers are deduplicated against actual pets and selected secondary anchors.

The control's existing cage candidates, scores, and flow pruning are untouched. New centers generate actual min-cut shapes using the existing radius range. For a projected center only, replace the originating cat's contribution with a blend of its current-distance kernel and a forecast of occupancy; all other pet terms are unchanged. The bound still uses the exact rating numerator. No extra forecast-specific motion or construction. Elapsed CPU guard is preserved.

| `CAT_ANCHORS` | Candidate rule |
| --- | --- |
| 0 | Disable projected candidates |
| 1 | Lead 2/4/6, EMA confidence at least .52, blend .63×confidence |
| 2 | More aggressive: confidence .40 and blend .82×confidence |
| 3 | Lead 2 only, confidence .52, blend .73×confidence |
| 4 | Same extra centers as mode 1, no projected occupancy bonus (geometric ablation) |

## Results

`train.csv`, fresh generator 28: control 366,075,606; mode 1 397,376,498 (+8.55%, 11/12/5); mode 2 341,002,357 (-6.85%, 9/11/8); mode 3 396,398,326 (+8.28%, 7/18/3); mode 4 373,927,987 (+2.15%, 1/25/2). Modes 1/2/3 had 26/39/13 projected cages win selection out of 1506/1944/767 extra cut evaluations. It is not a broad change in scoring, and outcomes are sensitive to a handful of substitutions.

## Reproduce

```sh
# Run from the repository root; tools is the absolute path to the official tools.
archive="$PWD/problems/ahc008/experiments/20261122"
work=$(mktemp -d)
g++ -std=c++23 "$archive/mincut-speed.single.cpp" -o "$work/control"
g++ -std=c++23 "$archive/cat-anchors/anchor.single.cpp" -o "$work/anchor"
cd "$work"
"$tools/target/release/gen" "$archive/cat-anchors/seeds.txt"
python "$archive/cat-anchors/compare.py" "$tools/target/release/tester" 0 28 train-repro.csv original 1 2 3 4
python "$archive/cat-anchors/compare.py" "$tools/target/release/tester" 28 48 holdout-repro.csv original 1 2 3
```

The seed stream is 80 distinct unsigned 64-bit values from Python Random(727491318). Last four were unused. Official tester run checks score and exactly 300 output actions; benchmark additionally records action hashes and counters.

`holdout.csv`, distinct fresh generator 48: control 798,574,255; mode 1 769,281,169 (-3.67%, 9/25/14; paired bootstrap 95% [-19.4%,+14.1%]); mode 2 815,041,615 (+2.06%, 19/13/16; [-17.2%,+25.9%]); mode 3 799,292,948 (+0.09%, 6/36/6; [-9.0%,+11.0%]). All runs completed 300 turns. Maximum wall time across held-out official tester invocations was 0.734 s. Median tester latency rose from 0.215 s to 0.288/0.291/0.243 s in modes 1/2/3.

`metrics.csv` separately resimulates the cases where any forecast-center cage was selected (mode 1: 26 cases, mode 3: 13), and all traces matched the benchmark action hashes. In mode 1, of 12 forecast cages that closed, zero trapped the source cat, although six trapped some pet. In mode 3 the corresponding counts were six completions, zero source cat, four some pet. The total count of cats in the largest human-holding component at turn 300 was six both before and after for each variant's respective changed subset. The heuristic generates different legal geometry, but forecasting a few moves ahead does not track where the animal will be at eventual closure. **Do not integrate.**

`anchor.single.cpp` is the complete experimental solver; `metrics.csv` is measured instrumentation output, archived separately.
