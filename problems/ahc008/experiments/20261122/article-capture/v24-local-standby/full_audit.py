"""Compare exact observed transitions in new and lost complete held cases."""
import csv,hashlib,json,sys,tempfile
from pathlib import Path
W=Path(__file__).resolve().parent.parent
H=W/'ahc008_focus_solver/problems/ahc008'
sys.path[:0]=[str(H),str(H/'experiments/20261122')]
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,flood
from benchmark import replay
from benchmark_cases import run
T=W/'ahc008_focus_assets/tools/target/release/tester'
P=Path(__file__).resolve().parent
exe={'baseline':W/'ahc008_terry_same_geometry/hybrid/source-release/frozen','article':P/'solve'}
ref={}
for r in csv.DictReader((P/'fresh100.csv').open()):ref.setdefault(r['case'],{})[r['model']]=r
assert len(ref)==100
def work(raw,row,exe):
 with tempfile.TemporaryDirectory() as d:
  p=Path(d);cmd=p/'cmd';trace=p/'trace';phase=p/'phase';local=p/'local'
  a,e,rc,_=run([str(T),'env','AHC008_ARTICLE_LOG='+str(phase),'AHC008_LOCAL_STANDBY_LOG='+str(local),str(exe)],raw,4.5)
  assert rc==0 and hashlib.sha256(a).hexdigest()==row['actions_sha256'],row['case']
  cmd.write_bytes(a)
  ar,er,r,_=run([str(T),sys.executable,H/'benchmark_cases.py','--playback',cmd,trace],raw,12.)
  assert (ar,er,r)==(a,e,0)
  actions=a.decode().splitlines(); moves=trace.read_text().splitlines()
  score,*_=replay(raw.decode(),moves,actions,detailed=True)
  assert score==int(row['score'])
  kinds,hist,_=observed(raw,moves)
  modes={int(z[0]):int(z[1]) for row in phase.read_text().splitlines() if (z:=row.split())}
  def area(h):
   ps,hs,w=h
   open_=ALL^sum(B[idx(q)] for q in w)
   u=flood(sum(B[idx(h)] for h in set(hs)),open_)
   ar=sum(flood(B[idx(h)],open_).bit_count() for h in hs)/len(hs)
   live=tuple(i for i,p in enumerate(ps) if u&B[idx(p)])
   return ar,live
  e=[area(h) for h in hist]
  cross=[]
  for at, (ar,live) in enumerate(e[1:],1):
   prev,last=e[at-1]
   if prev>=300 and ar<300:
    cross.append(dict(turn=at,before=prev,after=ar,mode=modes.get(at-1,0),walls=sum(c.islower() for c in actions[at-1]),
                      live_before=len(last),live_after=len(live),
                      newly_caught=sorted(set(last)-set(live))))
  return dict(whole=not e[-1][1],floor=e[-1][0],cross=cross,first_clear=next((i for i,(_,x) in enumerate(e) if not x),None),
              remain=[(i,kinds[i]) for i in e[-1][1]],snapshot=[(i,e[i][0],len(e[i][1])) for i in (160,225,250,275,300)],
              local=list(map(int,local.read_text().split())) if local.exists() else [])
out=[]
for name,a in ref.items():
 if a['baseline']['all_clear']==a['article']['all_clear'] and not (a['baseline']['all_clear']=='1' and (float(a['baseline']['mean_human_floor'])>=300)!=(float(a['article']['mean_human_floor'])>=300)):continue
 raw=(P/'fresh-m5-100/in'/name).read_bytes()
 assert all(hashlib.sha256(raw).hexdigest()==x['input_sha256'] for x in a.values())
 out.append(dict(case=name,**{k:work(raw,a[k],exe[k])for k in exe}))
 print(name,flush=True)
(P/'held-full-transitions.json').write_text(json.dumps(out,indent=2)+'\n')
