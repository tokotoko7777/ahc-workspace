# AHC008: stop spending walls after the final separation

Take the verified exact topology-cache revision
`be1cefb260f7f47b683f780eca2db2db603be3e0` as control. Do not change
geometry, guard selection, pet responses, release or tactical search.
Only after an actually emitted wall-changing turn, run a multi-source
flood from all people. If no animal shares any human component at the
next observed turn, permanently output only waits. Continue reading
and flushing each required interaction up to turn 300. There is no
forecast, no case heuristic and no RNG assumption.

**Why safe.** Neither a legal human step nor an animal step can cross an
existing wall component. Additional walls can only reduce each person's
reachable area and cannot let an animal into already clean area. After
the first all-clear state, staying keeps every actual current area,
and every per-person pet count remains zero. Relative to any legal
continuation of the same prefix, the exact integer final score cannot
decrease. Animal-only components, including all exact-four cages,
stay intact. The proof compares continuations of the same state:
the benchmark separately verifies the unchanged prefix even with
the controller's CPU budget checks. A final action that first clears
everything has nothing left to improve and creates no earlier trigger.

| Official v3, same exact prefix | Used dev30 | Distributed practice100 | Frozen independent generated100 |
|---|---:|---:|---:|
| Previous score | 746,775,771 | 2,116,997,192 | 1,916,659,829 |
| New score | 746,886,882 | 2,117,108,303 | 1,916,993,162 |
| Gain | +111,111 | +111,111 | +333,333 |
| Score wins/ties/losses | 1/29/0 | 1/99/0 | 1/99/0 |
| Cleared with ≥1 turn left | 17 | 29 | 33 |
| Old walls placed after first clear | 1 | 1 | 2 |
| Previous→new median tester seconds | .313573→.310214 | .337208→.329701 | .347370→.342264 |
| Previous→new max tester seconds | .457190→.503180 | .544342→.527633 | .533729→.539303 |

All 460 executions are legal and exact official/independent replay
agreed. The benchmark locates the true earliest all-clear observation
from the control trace and requires both code's trigger and the entire
prefix to match; no-trigger traces must be entirely identical. The
two 100-case control hashes and scores match the prior frozen ledgers.
Witnesses (zero-based observation turn, case): dev t283 / 0026
74,000,000→74,111,111; practice t214 / 0036 62,888,889→63,000,000;
independent t226 / 0050 57,777,778→58,111,111.
This is a small increase in the existing practice best, not evidence
of beating the original winner, and does not alter the capture rate.
No main or AtCoder submission changes.

```sh
D=problems/ahc008/experiments/20261122
H=problems/ahc008
g++ -std=c++23 "$D/parallel-banks-narrow-seam-cache.cpp" -o /tmp/ahc008-seam
g++ -std=c++23 "$D/parallel-banks-narrow-global-clear.cpp" -o /tmp/ahc008-clear
python3 "$D/parallel_banks_global_clear_bench.py" "$H" "$TESTER" /tmp/ahc008-seam /tmp/ahc008-clear "$PRACTICE100" /tmp/ahc008-clear-practice.csv --n 100
python3 "$D/parallel_banks_global_clear_compare.py" /tmp/ahc008-clear-practice.csv "$D/parallel-banks-narrow-rescue-practice100.csv.gz"
```

Use the same fresh seed list as the earlier frozen narrow comparison
for independent100; the C++23 files include the code's own optimization
pragma and need no flags beyond the C++ standard. Tested executable
SHA256 new `1386b30fc66cf194c61ce375e594b7dfd9f15ecece6bc9a50bc450e426234e67`.
