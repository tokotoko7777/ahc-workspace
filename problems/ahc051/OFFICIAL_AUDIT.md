# Frozen AHC051 historical-case audit

Solver freeze: `858e24331cd9b2b1ad0c114c91e72d341f552d18` (the default
reconvergent-splitter version). No solver, parameter, or candidate selection
changes were made after generation of the historical inputs. [Aggregate
manifest and result](official-audit-summary.json); no historical input/answer
files or per-case candidate scores are committed.

The [official statement](https://atcoder.jp/contests/ahc051/tasks/ahc051_a)
links the final [seeds.txt](https://img.atcoder.jp/ahc051/seeds.txt), including
its expected SHA-256. The [official standard generator/visualizer
pack](https://img.atcoder.jp/ahc051/jdd9gfQC.zip) has SHA-256
`7b6401fb897b9f6909934ebf41a802600c73c74c2cb8999d417fa00701daa054`.
Give the untouched seed file to its normal generator, with no options or
overrides. Before running the frozen solver, verify each of the 2,000 file IDs,
seeds and generated `N M K` headers against the [official case
metadata](https://img.atcoder.jp/ahc_standings/ahc051/input.csv). The manifest
records hashes of the ordered concatenated generated files and all data files.

Every answer must pass the official visualizer and independent integer
geometry/acyclicity and exact-rational probability replay. The elapsed
time measures standalone solver execution, not the two scoring programs.

Use the positive case minima across all archived users from the [official
score matrix](https://img.atcoder.jp/ahc_standings/ahc051/result.csv). Compute
integer round-to-nearest separately per case, then sum. Zero/negative archived
outcomes are non-scoring. Check the reconstructed winning account against the
[official ranked result](https://atcoder.jp/contests/ahc051/results/json).
Report both the fixed historic minima and a joint comparison using shared
minima recalculated to include the frozen solver.

## Reproduce

From the repository root at the committed audit revision, with the official
pack's `gen` and `vis` built in Rust release mode, fetch the three official
files linked above. Set `GEN`, `VIS`, `SEEDS`, `INPUTS`, `SCORES` to those
paths. The work directory may be temporary. The audit needs Python 3.10+
(`zip(strict=True)`) and g++ with C++23 support.

```sh
p=problems/ahc051
d=$(mktemp -d)
python scripts/bundle.py "$p/main.cpp" -o "$d/submission.cpp"
g++ -std=c++23 -O3 "$d/submission.cpp" -o "$d/solver"
python "$p/official-audit.py" "$SEEDS" "$INPUTS" "$SCORES" \
  "$GEN" "$VIS" "$d/solver" "$d/work" \
  --freeze-id 858e24331cd9b2b1ad0c114c91e72d341f552d18
```

Add `--prepare-only` to stop after preflight. Subsequent files in the work
directory are local diagnostics, not sources for further solver tuning.

## Result

| Relative score, higher is better | Historic case minima | Shared minima including the candidate |
|---|---:|---:|
| Frozen candidate | 310,855,698,519 | 310,855,698,519 |
| Historic #1, eijirou | 1,721,504,233,142 | 1,721,504,233,142 |

All 2,000 cases were legal and scored identically by the officials' standalone
visualizer and independent replay; maximum local solver time was 0.242450 s.
On 1,997 cases all processors had reachable paths. The other three remained
legal and lost the probability mass corresponding to unreachable processors.
The candidate set no new case minimum, so the two normalizations agree.
Hypothetical rank: 208 of 1,184, no ties. The historical winner was **not beaten**.
The audit did not send any submissions to AtCoder.
