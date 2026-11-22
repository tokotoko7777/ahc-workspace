"""Check archived v7, report paired exact isolation and actual TERRY stage usage."""
import csv,json,hashlib,statistics,sys
from collections import Counter
from pathlib import Path
here=Path(__file__).resolve().parent

def read(p,mode):
 return {r['case']:r for r in csv.DictReader(p.open()) if r['model']==mode}
def count4(rows,t=300):
 return sum(int(k[3])==4 and int(k[0])<=t for r in rows.values() for e in r['captures'].split(';') if e for k in [e.split(':')])
def stats(r):
 a=list(r.values());f=lambda r:float(r["mean_human_floor"])
 return dict(n=len(a),score=sum(int(r['score']) for r in a),
  full=sum(int(r['all_clear']) for r in a),
  full300=sum(int(r['all_clear']) and f(r)>=300 for r in a),
  full100=sum(int(r['all_clear']) and f(r)>=100 for r in a),
  mean_floor=statistics.mean(map(f,a)),
  residual=sum(int(r['final_reachable']) for r in a),
  four=count4(r),four100=count4(r,100),
  wall_med=statistics.median(float(r['wall_sec']) for r in a),
  wall_max=max(float(r['wall_sec']) for r in a))
def stage(rows):
 vals=[];completed=[]
 for r in rows.values():
  a,*p=r['terry'].strip().split(';')
  a=list(map(float,a.split())); assert len(a)==16 and a[2]==1
  vals.append(a)
  if p:completed.extend(int(c) for c in p[0].split())
 return dict(map_counts=dict(sorted(Counter(int(x[0]) for x in vals).items())),
   predicted_cases=sum(x[8]>0 for x in vals),
   nominal_install_med=statistics.median(x[3] for x in vals),
   scheduled_walls_issued=int(sum(x[4] for x in vals)),
   lane_handoff_cases=sum(x[6]>=0 for x in vals),
   lane_arrivals=sum(t>=0 for t in completed),
   late_scored_cases=sum(x[9]>0 for x in vals), late_pairs=int(sum(x[9] for x in vals)),
   late_cut_votes=sum(x[14]>0 for x in vals), late_actual_closures=int(sum(x[15] for x in vals)))
def report(s):
 q=here/f'terry-{s}.csv'
 a,b=read(q,'baseline'),read(q,'article')
 h=read(here.parent/'ahc008_integrated_v7'/f'v7-{s}.csv','article')
 assert len(a)==len(b)==len(h)==100 and a.keys()==b.keys()==h.keys()
 for case in a:
  for k in 'input_sha256 actions_sha256 score'.split():assert a[case][k]==h[case][k],(case,k)
  assert a[case]['input_sha256']==b[case]['input_sha256']
 c=lambda k,x:int(x[k]);d=lambda u,v,d: sum((int(b[k][d])-int(a[k][d]))*u>v for k in a)
 return dict(sha256=hashlib.sha256(q.read_bytes()).hexdigest(),baseline=stats(a),candidate=stats(b),
             clear_gained=d(1,0,'all_clear'),clear_lost=d(-1,0,'all_clear'),score_wins=d(1,0,'score'),
             same_actions=sum(a[k]['actions_sha256']==b[k]['actions_sha256'] for k in a),
             realized=stage(b))
if __name__=='__main__':
 print(json.dumps({s:report(s) for s in (sys.argv[1:] or ['practice100','independent100'])},indent=2))
