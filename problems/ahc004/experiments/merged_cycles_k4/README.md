# Retry four-letter overlaps only when five-letter overlaps fail the geometry gate

The original observed-cycle inventory and its geometry gate run unchanged.
Only when that gate fails, assemble complete lines using the five-letter
overlap strategy from the first merged-cycle arm. If the augmented set cannot
pass the existing 28-line / 75% exact-coverage gate, attempt a fresh four-letter
assembly and evaluate it against the original inventory. Never mix the two
assemblies or overwrite successful five-letter geometry input. Both attempts
and exact read rescoring share one 20 ms deadline. The solver's initializer
deadline and overall time limit are unchanged.

`partial_overlap` is the overlap accepted at the gate (zero if neither).
`partial_tries` records how many assemblies were attempted; `partial_ms`
measures their shared time, including coverage checking. `partial_novel`
includes novel candidate tests across all attempts and may count the same
string in both attempts; it is not a distinct output count.

```sh
python3 scripts/bundle.py problems/ahc004/experiments/merged_cycles_k4/submission_entry.cpp -o merged_retry.single.cpp
g++ -std=c++23 merged_retry.single.cpp -o merged_retry
```

The accompanying synthetic table uses 40 independent generated tori per
(length, count), no official input or seed. On L7, M650, its approximate gate
went from 16/40 with overlap five to 34/40 with overlap four. Novel false
complete lines remained 0.1 on average. This measures inventory and synthetic
gate eligibility (not achieved scores); matched official-distributed
benchmarking is separate. To reproduce that table:
```sh
cd problems/ahc004/experiments/merged_cycles
g++ -std=c++23 aug-driver.cpp -o aug-driver
sed 's/for k in \[5,6\]:/for k in [4]:/' cyclecheck.py | PYTHONHASHSEED=0 python3 -
```
