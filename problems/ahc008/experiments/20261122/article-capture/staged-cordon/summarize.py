"""Summarize complete paired AHC008 ledgers; strict input and exact parent trace."""
import csv, json, statistics,sys
from pathlib import Path
if len(sys.argv)!=3: raise SystemExit("usage: summarize.py this.csv exact-parent.csv")
def read(p):return list(csv.DictReader(Path(p).open()))
rows=read(sys.argv[1]);old=read(sys.argv[2])
a={r["case"]:r for r in rows if r["model"]=="baseline"}
b={r["case"]:r for r in rows if r["model"]=="article"}
old={r["case"]:r for r in old if r["model"]=="article"}
assert len(a)==len(b)==len(old)==30 and a.keys()==b.keys()==old.keys()
for k in a:
 for field in ('input_sha256','score'):
  assert a[k][field]==old[k][field]!=None
 assert a[k]['actions_sha256']==old[k]['actions_sha256']
 assert b[k]['input_sha256']==a[k]['input_sha256']

def metrics(d):
 counts={p:0 for p in range(1,6)}
 n100=0
 for r in d.values():
  for event in r['captures'].split(';'):
   if event:
    t,i,k,area,rr=map(int,event.split(':'))
    if area==4:
     counts[k]+=1;n100+=int(t<=100)
 m={k:sum(int(r[k]) for r in d.values()) for k in ('all_clear','score','final_reachable','final_exact4','residual_turn100','residual_turn160')}
 m.update(dict(four_by_species=counts,four_by100=n100,mean_floor=statistics.mean(float(r['mean_human_floor']) for r in d.values()),wide_clear=sum(int(r['all_clear'])>0 and float(r['mean_human_floor'])>=300 for r in d.values()),wall_med=statistics.median(float(r['wall_sec']) for r in d.values()),wall_max=max(float(r['wall_sec']) for r in d.values())))
 return m
cas=[]
for k in a:
 u,v=a[k],b[k]
 cas.append(dict(case=k,heads=(int(u['final_reachable']),int(v['final_reachable'])),score=(int(u['score']),int(v['score'])),floor=(float(u['mean_human_floor']),float(v['mean_human_floor'])),same_prefix=u['prefix100']==v['prefix100']))
z=[list(map(float,r['cordon'].split())) for r in b.values()]
cov=list(map(sum,zip(*z)))
print(json.dumps(dict(baseline=metrics(a),staged=metrics(b),same_prefix=sum(r['same_prefix'] for r in cas),cordon=cov,clear_gains=[r for r in cas if r['heads'][0]>0==r['heads'][1]],clear_losses=[r for r in cas if r['heads'][0]==0<r['heads'][1]],per_case=cas),indent=2))
