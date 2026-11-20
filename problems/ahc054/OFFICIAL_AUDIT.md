# Frozen AHC054 historical-case audit

The submitted-format solver was frozen at
`86ecac3a43ec178c4c694db07c6d0c2163e9812b`: induced spine plus blind spurs.
The audit does not change it or use per-case final results to tune it.
Only the runner and the [aggregate summary](official-audit-summary.json) are
committed, not any historical cases, answers, or own per-case scores.

The [official statement](https://atcoder.jp/contests/ahc054/tasks/ahc054_a)
specifies 3,000 system tests, and links the [published seeds](https://img.atcoder.jp/ahc054/seeds.txt)
with SHA-256. The [official v2 tools](https://img.atcoder.jp/ahc054/YDAxDRZr_v2.zip)
have ZIP SHA-256 `1410408e787880e574e7b0a709798c9682e45be7358da2c134ea386b4d7c33e3`.
Give the untouched seed file to the standard generator. Before running the
solver on any final case, verify all filenames and seed order against the
[official input metadata](https://img.atcoder.jp/ahc_standings/ahc054/input.csv),
and compare regenerated forest side `N` and exact initial tree count `K` on
every case. This matches the official procedure; the ordered case byte hash
is recorded to identify the local regeneration.

The interactive official tester, given each complete generated case including
the official secret priority permutation, feeds the solver only the public
protocol. Independent replay rechecks each planting and continued connectivity,
ray reveals, exact destination choice and movement, and the score. Time
measurements include the official tester, as the statement requires, but not
the separate replay. Source and binary are not changed between runs.

The [official score matrix](https://img.atcoder.jp/ahc_standings/ahc054/result.csv)
has 1,190 archived users and a score per final case. Compute each historical
maximal valid score and verify the resulting historical #1 identity against
[official rank metadata](https://atcoder.jp/contests/ahc054/results/json).
For fair addition of the candidate use new shared maxima whenever it beats an
archived case maximum, with separate integer rounding of each case relative
score. Fixed historic maxima are reported only as a secondary diagnostic.

## Reproduce

From the repository root, set `GEN`, `TESTER`, `SEEDS`, `INPUTS` and `SCORES`
to the paths above. Build the official tools in Rust release mode. Use the
C++23 one-file bundle (built in this run with g++ 14.2.0):

```sh
p=problems/ahc054
d=$(mktemp -d)
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/solver"
python "$p/official-audit.py" "$SEEDS" "$INPUTS" "$SCORES" \
  "$GEN" "$TESTER" "$d/solver" "$d/work" \
  --freeze-id 86ecac3a43ec178c4c694db07c6d0c2163e9812b
```

Use `--prepare-only` to stop after the metadata preflight.

## Result

All 3,000 cases were legal and scored identically in the official tester and
independent replay. Maximum tester-inclusive wall time was 0.196777 s.
Summed raw moves: 3,042,194. The candidate set no new case maximum, so the
fixed and common combined normalization coincide.

| Relative score, higher better | Historic maxima | Common maxima |
|---|---:|---:|
| Frozen candidate | 419,674,926,324 | 419,674,926,324 |
| Historic #1, saharan | 2,409,003,089,874 | 2,409,003,089,874 |

Hypothetical rank 505 / 1,191, no ties. The historical winner was **not
beaten**. Nothing was submitted.
