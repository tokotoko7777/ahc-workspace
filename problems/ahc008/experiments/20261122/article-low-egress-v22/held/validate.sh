#!/usr/bin/env bash
set -euo pipefail
root=$(cd -- "$(dirname -- "$0")/.." && pwd)
cd "$root/ahc008_integrated_v22"
export PYTHONPATH="$root/ahc008_integrated_v11:$root/ahc008_bridge_audit:$root/ahc008_focus_solver/problems/ahc008/experiments/20261122"
harness="$root/capture-first-analysis"; tester="$root/ahc008_focus_assets/tools/target/release/tester"; base="$root/ahc008_integrated_v7"
for k in practice independent; do
 if [[ $k == practice ]]; then cases="$root/ahc008_focus_assets/tools/in"; n=34; else cases="$root/ahc008-article-bench/full-capture-independent100/in"; n=32; fi
 name="$k-low$n"; mkdir -p "$name"
 python - "$cases" "$name" "$n" <<'PY'
import pathlib,sys
cases,out,n=pathlib.Path(sys.argv[1]),pathlib.Path(sys.argv[2]),int(sys.argv[3])
files=sorted(cases.glob('*.txt'))
assert len(files)==100
sel=[]
for p in files:
 words=p.read_text().split(); m=int(words[1+3*int(words[0])])
 if m<=6:
  sel.append(p.name)
  target=out/p.name
  if target.exists(): assert target.resolve()==p.resolve()
  else: target.symlink_to(p.resolve())
assert len(sel)==n
PY
 out="v22-$name"
 python -u bench_v22.py "$harness" "$tester" "$base/integrated" "$PWD/integrated" "$name" "$out.csv" --n="$n" >"$out.log"
 python ../ahc008_integrated_v11/summarize.py "$out.csv" >"$out-summary.json"
 python - "$base/v7-$k"100.csv "$out.csv" "$n" <<'PY'
import csv,sys
n=int(sys.argv[3])
old={r['case']:r for r in csv.DictReader(open(sys.argv[1])) if r['model']=='article' and int(r['humans'])<=6}
rows=list(csv.DictReader(open(sys.argv[2])))
assert 2*n==len(rows) and len(old)==n
assert set(x['case'] for x in rows)==old.keys()
assert all(all(old[x['case']][f]==x[f] for f in ('actions_sha256','input_sha256','score','all_clear','final_reachable','exact4_first')) for x in rows if x['model']=='baseline')
print(f'All {n} affected histories match the old full100 v7 control')
PY
done
