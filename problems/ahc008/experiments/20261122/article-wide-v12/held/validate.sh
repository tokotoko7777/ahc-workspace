#!/usr/bin/env bash
set -euo pipefail
root=$(cd -- "$(dirname -- "$0")/.." && pwd)
cd "$root/ahc008_integrated_v12"
candidate=$(realpath "$1")
name=$2
[[ $name =~ ^v1[23]$ ]]
export PYTHONPATH="$root/ahc008_bridge_audit:$root/ahc008_focus_solver/problems/ahc008/experiments/20261122${PYTHONPATH:+:$PYTHONPATH}"
tester="$root/ahc008_focus_assets/tools/target/release/tester"
harness="$root/capture-first-analysis"
pred="$root/ahc008_integrated_v7"
for kind in practice independent; do
  if [[ $kind == practice ]]; then
    cases="$root/ahc008_focus_assets/tools/in"
  else
    cases="$root/ahc008-article-bench/full-capture-independent100/in"
  fi
  out="$name-$kind"100
  python -u bench_v12.py "$harness" "$tester" "$pred/integrated" "$candidate" "$cases" "$out.csv" --n=100 >"$out.log"
  python ../ahc008_integrated_v11/summarize.py "$out.csv" >"$out-summary.json"
  python - "$pred/v7-$kind"100.csv "$out.csv" <<'PY'
import csv,sys
old={r['case']:r for r in csv.DictReader(open(sys.argv[1])) if r['model']=='article'}
new={r['case']:r for r in csv.DictReader(open(sys.argv[2])) if r['model']=='baseline'}
assert len(old)==len(new)==100 and old.keys()==new.keys()
assert all(all(a[k]==new[c][k] for k in ('input_sha256','actions_sha256','score','all_clear','final_reachable','exact4_first')) for c,a in old.items())
print('baseline historic comparison: all 100 input, actions, score and isolation exact')
PY
done
