# Frozen soft direction cue on rescue160 (dev30; not adopted)

Port the unchanged movement cue from the old stay225 direction archive
(`2ab39859...`). At t101 or later, use only a dog's/cat's most recently
observed first step near eligible prepared mouths to softly multiply its
bank-guard ranking by 1.6 inward or 0.5 outward, otherwise 1.0. Immediate
legal paired captures have precedence. The 3×12 geometry, builders,
persistent zones, t160 >4 rescue rule and ordinary tactical/terminal
remain fixed. A changed t160 count can change the actual switching time,
not the rule. No combination with width, irregular overlays or graph radius.
Use `5 1 1` for this experimental mode; no-args still frozen rescue.
The direction hypothesis was motivated by [simanman's authorized narrative](https://simanman.hatenablog.com/entry/2022/02/28/074838)
and our earlier legitimate exact trace analysis; no external code.

| Old dev30, official v3 + exact replay | Rescue | Direction + rescue |
|---|---:|---:|
| Score | 697433161 | 698296004 |
| Direct exact-four bank animals | 199 | 205 |
| Final exact-four animals | 205 | 208 |
| Residual at t160 | 246 | 238 |
| Cases switched at t160 | 24 | 26 |
| Final reachable residual | 40 | 63 |
| Fully clear cases | 14 | 12 |
| Pet-free humans | 113 | 112 |
| Distinct clean human area sum | 5026 | 5507 |
| Actual post-release 4–48 cell captures | 53 | 71 |
| Of those nonrectangular | 48 | 70 |
| Median/max tester seconds | .359/.446 | .347/.574 |

W/T/L 11/8/11 and all 60 legal. Exact action bytes through turn 100
inclusive match in all 30 pairs, and disabled cue matches all 30 frozen
rescue full hashes. Direct per-species [cow/pig/rabbit/dog/cat]:
[18,34,38,70,39] → [20,31,43,70,41]. Inward/outward weighted candidate
applications 1492/1364 (not independent animals). The increased irregular
trap count is actual topology at the moment of separation, including all
ordinary fallback actions on the resulting branch, not a direction-specific
cut function or evidence of general improvement. The tiny aggregate score
gain does not offset worse reliability; no fresh/official rerun, no
promotion. Existing research standalone is unchanged.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-direction-rescue.cpp "$D/parallel-banks-rescue-research.cpp" "$D/parallel-banks-direction-rescue.patch"
g++ -std=c++23 /tmp/ahc008-direction-rescue.cpp -o /tmp/ahc008-direction-rescue
python "$D/parallel_banks_direction_rescue_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-direction-rescue "$DEV30" /tmp/direction-rescue.csv --frozen-control-csv "$D/parallel-banks-rescue-dev30.csv"
```
