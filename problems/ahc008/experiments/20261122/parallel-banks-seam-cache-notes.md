# AHC008 exact late-separator topology memo (2026-11-22)

This is a performance-only variant of narrow rescue (immutable base commit
`3fbe0d94d8a5602d2d2063be2ff159dfe803fd4e`). The two-stage capture/fallback
policy, source geometry, scoring, cut selection, human staffing, legality
and pet model are all unchanged. Do not confuse this with the earlier
experimental cut or occupancy changes. It does not close the gap to
6,263,333,336 and is not a result on the original competition cases.

The existing late-line ranking tests many contiguous segments against the
same passable wall board. Cache the connected bitboard pieces in the
original flood order for each (static line index, start, end) in the current
wall generation and connected root. A memo entry of length zero records a
non-separation. Every query still counts the actually observed pets in
the pieces and evaluates the original score; no action values, occupancy
or paths are cached. Actual wall bytes and original connected root guard
invalidation. The executor's final fresh flood still produces cell labels.
Hash deduplication, arbitrary wiggle paths (inactive in this production
variant) and all other capture behavior are unchanged.

| Exact baseline and variant, sequential tester v3 | Generated dev 30 | Official distributed practice 100 | Frozen independent generated 100 |
|---|---:|---:|---:|
| Scores, both arms | 746,775,771 | 2,116,997,192 | 1,916,659,829 |
| All action bytes same | 30/30 | 100/100 | 100/100 |
| Median wall seconds, previous → memo | .358542→.317890 | .375712→.335552 | .377173→.344591 |
| Paired median relative change | −10.66% | −10.21% | −10.68% |
| Cases faster | 28/30 | 93/100 | 92/100 |
| Max wall seconds, previous → memo | .599441→.495153 | .619746→.540728 | .665439→.530256 |
| Cache hit count / ranking queries | 917,338 / 1,494,272 | 3,845,154 / 5,998,171 | 4,018,648 / 6,347,398 |

All 460 solver runs were legal and their whole actions and scores also
match independent replay. On both 100 cohorts every prior frozen narrow
hash and score is identical, not just the new A/B. Peak cached component
capacity was 8,192 on dev, 4,096 on either 100; each is a 30-row 32-bit
mask plus area. The preallocated exact (line,start,end) table adds at most
~2.2 MB on a 30×30 board; the component pool adds ~1.1 MB at observed peak.
These are storage bounds from capacities/types, not measured RSS.
Times include the same official tester path and diagnostic logs on the
memo arm, not AtCoder server CPU times. No timed tests were run in parallel
for this cache comparison.

Build/reproduce in repository root with existing extracted official v3
tools. GNU C++23 source already has its real in-file O3 pragma; no
compiler optimization flags are required. The second solver's optional
legacy `5 1 1` flags match no-argument geometry and operation. The bench
sets the optional cache instrumentation file only for the new program.

```sh
D=problems/ahc008/experiments/20261122
H=problems/ahc008
g++ -std=c++23 "$D/parallel-banks-narrow-rescue-research.cpp" -o /tmp/ahc008-narrow
g++ -std=c++23 "$D/parallel-banks-narrow-seam-cache.cpp" -o /tmp/ahc008-seam-cache
python3 "$D/parallel_banks_seam_cache_bench.py" "$H" "$TESTER" /tmp/ahc008-narrow /tmp/ahc008-seam-cache "$PRACTICE100" /tmp/ahc008-seam-practice.csv --mixed --case-count 100
python3 "$D/parallel_banks_seam_cache_compare.py" /tmp/ahc008-seam-practice.csv "$D/parallel-banks-narrow-rescue-practice100.csv.gz"
```

For independent100 use the previous frozen narrow-rescue seed list and
official generator, then the matching gzip ledger for comparison. The two
legacy labels inside the new ledgers are `rescue3` (no-arg narrow control)
and `narrow_rescue` (memo), retained for compatibility with the scripts;
both actually use narrow policy. Each benchmark records exact inputs and
action hashes as well as measured times. Validated local executable SHA256:
reference `85d5e63eabef79c74d1c0fe7631c88231ae008121a7605d33cef3f69419c4ba4`;
memo `325d1dfef4fd2a0950cd30715d3e1832405d56687f2ef4ecf8e5eb89165c2f6a`.
