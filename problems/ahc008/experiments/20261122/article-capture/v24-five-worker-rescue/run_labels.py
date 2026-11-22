"""Offline score range of six fixed v24 layouts; not a runtime layout policy."""
from __future__ import annotations
from pathlib import Path
import csv,hashlib,sys,json,os
root=Path.cwd(); out=Path(__file__).resolve().parent
sys.path[:0]=[str(root/'ahc008_terry_integration')]
from bench import play,measure_history
h=root/'ahc008_focus_solver/problems/ahc008'
sys.path[:0]=[str(h),str(h/'experiments/20261122')]
from benchmark_cases import run
from benchmark import replay
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,flood
from statistics import mean
tester=root/'ahc008_focus_assets/tools/target/release/tester'
exe=Path(sys.argv[1]).resolve()
p=root/'ahc008_v24_five_retention'
cases=sorted((p/'m5-dev30/in').glob('*.txt'))
ref={r['case']:r for r in csv.DictReader((p/'dev30.csv').open()) if r['model']=='baseline'}
def default_diag(raw):
 q=list(map(int,raw.split()))
 n=q[0]; ps=[(q[j]-1,q[j+1]-1) for j in range(1,3*n,3)]
 j=1+3*n;m=q[j]
 hs=[(q[k]-1,q[k+1]-1) for k in range(j+1,j+1+2*m,2)]
 assert m==5
 num=[0.]*6
 for z in range(2):
  for a0,a in enumerate((4,8,12)):
   i=3*z+a0
   for r,c in ps:
    x,y=(c,r) if z else (r,c)
    if not 2<=x<=26:continue
    d=min(abs(y-(a+7*b+k)) for b in range(2) for k in range(2,6))
    num[i]+=1./(1.+.8*d)
   for b in range(3):
    r,c=(a+7*b,14) if z else (14,a+7*b)
    n=min(abs(x-r)+abs(y-c) for x,y in hs)
    num[i]-=.045*n
 return max(range(6),key=lambda i:num[i])
os.environ["AHC008_M5_LAYOUT_LOG"]=str(out/"effective-layout.csv")
rows=[]
for f in cases:
 raw=f.read_bytes(); r=ref[f.name]
 hh=hashlib.sha256(raw).hexdigest()
 assert r['input_sha256']==hh
 for layout in range(6):
  q=play(str(tester),str(exe),[f'--m5-layout={layout}'],raw,run,replay,observed,
      lambda hist,kinds,release:measure_history(hist,kinds,B,ALL,idx,flood,release),h)
  geometry=(out/'effective-layout.csv').read_text().strip().split(',')
  assert int(geometry[1])==layout and int(geometry[-1])==layout,geometry
  rows.append(dict(case=f.name,input_sha256=hh,layout=layout,baseline_choice=default_diag(raw),**q))
  with (out/'labels.csv').open('w') as o:
   w=csv.DictWriter(o,list(rows[0]));w.writeheader();w.writerows(rows)
 print(f.name,[(r['layout'],r['score'],r['all_clear']) for r in rows[-6:]],flush=True)
assert len(rows)==6*len(cases)==180
sets={f.name:[x for x in rows if x['case']==f.name] for f in cases}
best=[max(q,key=lambda x:x['score']) for q in sets.values()]
same=[next(x for x in q if x['layout']==x['baseline_choice']) for q in sets.values()]
summary=dict(
 score_baseline_original=sum(int(x['score']) for x in ref.values()),
 score_baseline_forced=sum(x['score'] for x in same),
 score_oracle=sum(x['score'] for x in best),
 default_exact_action_count=sum(x['actions_sha256']==ref[x['case']]['actions_sha256'] for x in same),
 whole_possible=sum(any(x['all_clear'] for x in q) for q in sets.values()),
 useful_possible=sum(any(x['all_clear'] and x['mean_human_floor']>=300 for x in q) for q in sets.values()),
 oracle_raw_full=sum(x['all_clear'] for x in best),
 oracle_useful_full=sum(x['all_clear'] and x['mean_human_floor']>=300 for x in best),
 median_baseline=sum(x['score'] for x in same)/30,
 layout_scores=[sum(x['score'] for x in rows if x['layout']==i) for i in range(6)],
 max_runtime=max(x['wall_sec'] for x in rows))
(out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
print(json.dumps(summary,indent=2))
