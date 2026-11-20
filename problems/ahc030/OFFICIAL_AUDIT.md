# AHC030: frozen solver on the reconstructed final corpus

Candidate: unchanged exact active-drill solver
`8234cc4fdd41ddc7ed1f7dae967b83605eeda289`, same tree as public branch
`ff4f0b02b3b5d35bb7a63748604649304e7c5d15`. The separate weighted-survey
experiment was not used. No external submissions were made.

## Source and identity

- [Statement](https://atcoder.jp/contests/ahc030/tasks/ahc030_a),
  [published system seeds](https://img.atcoder.jp/ahc030/seeds.txt),
  [official local tools](https://img.atcoder.jp/ahc030/awGA15Om_v3.zip):
  the 3,000 published seeds went unchanged to the official generator. Their
  order matches every row in the official
  [input metadata](https://img.atcoder.jp/ahc_standings/ahc030/input.csv);
  generated N, M, eps and summed shape sizes match all 3,000 rows.
  The archive does not publish a case-content hash, so this is reproduction
  by official tools, not a byte comparison with a separate hidden copy.
- The historical per-case official minima come from the
  [official raw-score archive](https://img.atcoder.jp/ahc_standings/ahc030/result.csv).
  Column order matches the [official viewer](https://img.atcoder.jp/ahc_standings/index.html).
  Sum recomputation over all 1,132 rows reproduces **terry_u16** in #1 with
  **2,140,937,483,257**, matching the top name in
  [official results](https://atcoder.jp/contests/ahc030/results/json)
  and the total in the [official archive packaged by ALE-Bench](https://huggingface.co/datasets/SakanaAI/ALE-Bench/blob/main/ahc030.zip).
- Compare both scores using the historical per-case minimum and again using
  the minimum of that value and the candidate score, recomputing all rows.
  Positive ratios round half up by `(2*10^9*minimum + raw)//(2*raw)`;
  official invalid negative values contribute zero. The fixed historical
  diagnostic can produce more than 10^9 on any better-than-history case;
  the joint baseline never does.

Both an official interactive test and independent coordinate, support,
operation-limit and cost replay must agree on each absolute score. All
aforementioned checks validate at runtime; an incomplete or invalid run
does not produce a final summary.

## Result

| Common per-case minima | Candidate | Historical winner terry_u16 |
| --- | ---: | ---: |
| Archived minima unchanged | 323,222,495,387 | 2,140,937,483,257 |
| Include candidate in recomputed minima | 323,222,495,387 | 2,140,937,483,257 |

**Did not beat first place.** The candidate did not improve any archived
case minimum; hypothetical rank 171 of 1,133, no ties. All 3,000 were
legal and solved on the first guess; absolute score sum 196,158,000,000
and maximum wall time 0.068358 s including tester.

## Reproduction

From repository root, save the three official files listed above and set
`seedfile`, `input_csv`, `result_csv`, and `tools_dir` to the appropriate
absolute paths. Official zip SHA-256:
`a4fb47c576734b848977db04cd61ce3192b57b15ff3e5fbea116a5fc64be7f7c`.

```sh
(cd "$tools_dir" && cargo build --locked --release)
work=$(mktemp -d)
python scripts/bundle.py problems/ahc030/main.cpp -o "$work/solver.cpp"
g++ -std=c++23 -O3 "$work/solver.cpp" -o "$work/solver"
python problems/ahc030/official-audit.py "$seedfile" "$input_csv" "$result_csv" \
  "$tools_dir/target/release/gen" "$tools_dir/target/release/tester" \
  "$work/solver" "$work/audit"
```

Measured on one Linux worker pinned to CPU8, g++14.2.0; wall time includes
tester. Bundled-source SHA-256:
`24826c08cf1ef6a679f1f3adde3a5b69edaa70c585fc02f26b123b3ad7d3191a`.
See [summary](official-audit-summary.json) for archive, input, solver and
tool hashes. Neither final cases nor individual final scores are committed.
