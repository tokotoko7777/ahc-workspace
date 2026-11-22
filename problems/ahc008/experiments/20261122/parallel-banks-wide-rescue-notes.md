# Full-width paired banks with rescue160 (development only)

Question: does full-width coverage, which improved compact capture in the
old stay225 arm but hurt score, also improve the *unchanged* rescue rule?
Matched replay uses the pre-existing visible M>=7 development cohort of 30
and eight M<7 identity cases. Seeds in
`parallel-banks-wide-enriched-seeds.txt`; no practice or rescue-frozen100
tuning. No AtCoder submission or main changes.

The experimental binary has the frozen 3 x 12 rescue160 policy as default;
flags `5 1 1` enable 4 x 12 iff M>=7. The fourth bank adds one service
lane spanning the board, keeps both open crossbars, and uses the same exact
four inner cells with simultaneous paired gates. Stage guard assignment
starts for the first bank with 5 ready; the global expansion threshold is
24/36 -> 32/48, and hard full assignment still t101. In the wider bank,
M7/8 uses a persistent paired duplicate on lanes 1 and 2; M9 two pairs on
0/1 and 3/4; M10 all five lanes duplicated. The two-zone scheduler uses
the same values and exact nonoverlap choice, extended from six to eight
bank-zone slots. Rescue still hands off at t160 iff more than four pets
are actually reachable; otherwise stays to t225. Both builders/guards
and parent action are unchanged on the low-worker policy.

| 30 paired cases, M>=7 | 3-bank rescue | 4-bank rescue |
|---|---:|---:|
| Official v3 score | 575482884 | 651178350 |
| Direct atomic exact 4 captures | 201 | 220 |
| Final exact-four-pen animals | 207 | 227 |
| Actual t160 residual | 271 | 258 |
| Cases that switched at t160 | 29 | 26 |
| Final residual | 62 | 63 |
| All-clear cases | 10 | 11 |
| Cleared people | 105 | 111 |
| Distinct human-safe area sum | 4059 | 4459 |
| Median/max sec | .389/.577 | .500/.876 |

W/T/L 19/0/11. Direct counts cow/pig/rabbit/dog/cat:
[36,30,29,62,44] -> [40,43,32,63,42]. Ready at t101 averaged 35.07 ->
40.73 in absolute numbers, but ready fraction declines. Final residual
did not improve overall despite better t160 capture; one avoid-early-rescue
benefit does not imply broad improvements in every stratum. M-stratum
aggregate score changes 7/8/9/10 workers (+44.27m / -10.38m / +10.71m /
+31.09m, n10/8/6/6) are descriptive, not gating rules. All 76 official
test runs were legal and exactly verified by recorded playback and an
independent scorer; both crossbars and five possible service lanes stay
connected through each bank stage. All 8 low-worker full action traces
and geometry were byte-identical. Development evidence only; no promotion.

Reproduce using the official v3 release and the same input directories
from the earlier wide experiment. `HARNESS` provides benchmark.py and
benchmark_cases.py. The bench checks physical isolation to exactly the
four described cells for every direct atomic capture.

```sh
D=problems/ahc008/experiments/20261122
patch -s -o /tmp/ahc008-wide-rescue.cpp "$D/parallel-banks-rescue-research.cpp" "$D/parallel-banks-wide-rescue.patch"
g++ -std=c++23 "$D/parallel-banks-rescue-research.cpp" -o /tmp/ahc008-rescue
g++ -std=c++23 /tmp/ahc008-wide-rescue.cpp -o /tmp/ahc008-wide-rescue
python "$D/parallel_banks_wide_rescue_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-rescue /tmp/ahc008-wide-rescue "$WIDE_DEV/eligible" /tmp/eligible.csv
python "$D/parallel_banks_wide_rescue_bench.py" "$HARNESS" "$TESTER" /tmp/ahc008-rescue /tmp/ahc008-wide-rescue "$WIDE_DEV/low" /tmp/low.csv --case-count 8 --low-workers
```

## Frozen untouched uniform100: reject

The executable, parameters and visible M>=7 inclusion rule were frozen
before creating this independent official-generator cohort (seed list in
`parallel-banks-wide-rescue-fresh100-seeds.txt`; same dev binary SHA256
`36bcfbb8e444a4f27183a0a13593263b36d4b50a0db2aca2d1dc62ac41d68cc4`).
All 200 tester runs were legal, replay byte-identical, exact scores matched.

| Fresh uniform100 | 3-bank rescue | 4-bank when M>=7 |
|---|---:|---:|
| Official v3 sum, all 100 | 1777795382 | 1584247350 |
| Official v3 sum, 64 eligible only | 1425495183 | 1231947151 |
| Direct, atomic exact-four captures | 670 | 686 |
| Final exact-four animal count | 688 | 698 |
| t160 reachable residual | 875 | 866 |
| Final residual | 243 | 288 |
| Cases fully clear | 32 | 22 |
| Distinct clean area sum | 10603 | 8717 |
| Median/max wall seconds | .380/.612 | .469/1.020 |

Eligible W/T/L 27/0/37, clear-case wins/losses 13/23. Thirty-six low-worker
cases have exactly matching complete action bytes and selected geometry;
frozen verification does not infer equivalence from scores alone. The
development aggregate improvement did not replicate; after cutoff more
animals remained reachable and safe area fell despite increased exact-four
capture. The denser geometry and different trajectories reach different
tactical states. This A/B does not identify their separate causal effects.
Keep original rescue default; do not retune by M-stratum or run official
distributed100 for this rejected branch. Score delta uncertainty is
descriptive bootstrap on matched cases, not evidence of improvement.

Fresh replay: use the accompanying seed file as the sole argument to the
official generator, then replace the development input directory with the
generated `in` and pass `--mixed --case-count 100`. JSON reproduces from
the CSV with `parallel_banks_wide_rescue_summary.py`.
