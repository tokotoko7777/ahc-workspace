import csv
import importlib.util
from pathlib import Path
import re
import statistics
import subprocess
import tempfile
import time
from collections import Counter
here=Path(__file__).resolve().parent
import argparse
parser=argparse.ArgumentParser()
for key in ("inputs","parent_a","parent_b","vis","output"): parser.add_argument(key,type=lambda p:Path(p).resolve())
parser.add_argument("--recombine",required=True,type=lambda p:Path(p).resolve())
args=parser.parse_args()
inputs,pa,pb,vis=args.inputs,args.parent_a,args.parent_b,args.vis
spec=importlib.util.spec_from_file_location("solver_audit",here.parents[2]/"benchmark.py")
module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
score_case=module.score_case
dest=args.output
dest.mkdir(parents=True,exist_ok=True); dest=dest/'boards'; dest.mkdir(exist_ok=True)
st=time.monotonic()
cases=sorted(inputs.glob('*.txt'))
assert [p.name for p in cases]==[f'{i:04}.txt' for i in range(100)]
group_hist=Counter(); largest_hist=Counter()
with tempfile.TemporaryDirectory() as tmp, (args.output/'results.csv').open('w',newline='') as f:
 w=csv.writer(f)
 w.writerow('case score fast rebuild maximum gain found empty raw aligned giant isolated exact fallback constant impossible work elapsed_s component_sizes'.split())
 all_rows=[]
 for p in cases:
  input_text=p.read_text()
  t=time.monotonic()
  proc=subprocess.run([args.recombine,pa/p.name,pb/p.name],input=input_text,text=True,capture_output=True,check=True,timeout=2)
  elapsed=time.monotonic()-t
  stats=dict(x.split('=') for x in proc.stderr.split())
  sizes=[int(i) for i in stats['sizes'].split(';') if i]; group_hist.update(sizes); largest_hist.update([(sizes or [0])[0]])
  raw,aligned,exact,fallback,constant,imp,work=[int(stats[x]) for x in ('raw','aligned','exact','fallback','constants','impossible','work')]
  score,found,empty=score_case(input_text,proc.stdout)
  assert found==int(stats['predicted'])
  (dest/p.name).write_text(proc.stdout)
  j=subprocess.run([vis,p,dest/p.name],cwd=tmp,capture_output=True,text=True,check=True)
  assert not j.stderr and re.fullmatch(r'Score = (\d+)\s*',j.stdout) and int(j.stdout.split('=')[1])==score,(p,j)
  a,b=(score_case(input_text,(parent/p.name).read_text())[0] for parent in (pa,pb))
  best=max(a,b); assert score>=best
  row=(p.name,score,a,b,best,score-best,found,empty,raw,aligned,max(sizes or [0]),sizes.count(1),exact,fallback,constant,imp,work,elapsed,stats['sizes'])
  w.writerow(row); all_rows.append(row)
 print('totals score,a,b,best,gain',*[sum(r[i] for r in all_rows) for i in range(1,6)])
 print('wins vs max',sum(r[5]>0 for r in all_rows),'maxcase gain',max(r[5] for r in all_rows),'max offline per case',max(r[17] for r in all_rows))
 print('raw,aligned,giant median',*[statistics.median(r[i] for r in all_rows) for i in (8,9,10)])
 print('component buckets',[sum(n for k,n in group_hist.items() if l<=k<=h) for l,h in [(1,1),(2,4),(5,16),(17,100),(101,400)]])
 print('largest ranges',[sum(n for k,n in largest_hist.items() if l<=k<=h) for l,h in [(0,0),(1,16),(17,100),(101,200),(201,300),(301,400)]])
 print('giant is all changed',sum(r[9]==r[10] for r in all_rows),'coverage totals constant/ impossible',sum(r[14] for r in all_rows),sum(r[15] for r in all_rows),'secs',time.monotonic()-st)
