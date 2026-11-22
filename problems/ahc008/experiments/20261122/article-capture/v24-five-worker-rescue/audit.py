"""Read-only diagnostics on exact v24; no new solver policy or target selection."""
from pathlib import Path
import csv,json,hashlib,statistics,tempfile,sys,subprocess
W=Path.cwd()
H=W/'ahc008_focus_solver/problems/ahc008'
sys.path[:0]=[str(H),str(H/'experiments/20261122')]
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,flood
from benchmark import replay
from benchmark_cases import run
T=W/'ahc008_focus_assets/tools/target/release/tester'
EXE=W/'ahc008_terry_same_geometry/hybrid/source-release/frozen'
p=W/'ahc008_v24_five_retention'
ref={r['case']:r for r in csv.DictReader((p/'dev30.csv').open()) if r['model']=='baseline'}
out=[]
for f in sorted((p/'m5-dev30/in').glob('*.txt')):
 raw=f.read_bytes();known=ref[f.name]
 assert hashlib.sha256(raw).hexdigest()==known['input_sha256']
 with tempfile.TemporaryDirectory() as d:
  pth=Path(d);a=pth/'a'; z=pth/'trace'; ph=pth/'phase'
  actions,err,rc,_=run([str(T),'env','AHC008_ARTICLE_LOG='+str(ph),str(EXE)],raw,4.5)
  assert rc==0; a.write_bytes(actions)
  b,newerr,newrc,_=run([str(T),sys.executable,H/'benchmark_cases.py','--playback',a,z],raw,12.)
  assert (b,newerr,newrc)==(actions,err,0)
  moves=z.read_text().splitlines()
  score,*_=replay(raw.decode(),moves,actions.decode().splitlines(),detailed=True)
  assert err.decode().strip()==f'Score = {score}'
  kinds,hist,_=observed(raw,moves)
  mode={int(x[0]):int(x[1]) for r in ph.read_text().splitlines() if (x:=r.split())}
  assert len(hist)==301
  events=[]
  prev=None; first=[]
  for t,(pets,humans,walls) in enumerate(hist):
   op=ALL^sum(B[idx(w)] for w in walls)
   hb=sum({B[idx(h)] for h in humans})
   root=flood(hb,op)
   uncovered=tuple(i for i,p in enumerate(pets) if root&B[idx(p)])
   reach=root.bit_count()
   area=sum(flood(B[idx(h)],op).bit_count() for h in humans)/len(humans)
   if prev is not None:
    before,last,w0,area0=prev
    if t>140 and (reach<before-10 or uncovered!=last):
     events.append(dict(t=t,mode=mode.get(t-1,0),added=len(walls)-w0,before=before,after=reach,avg_before=area0,avg_after=area,remaining=len(uncovered),heads=[i for i in last if i not in uncovered]))
    if prev[0]>=300>reach and not first:first=[t,mode.get(t-1,0),before,reach,len(uncovered)]
   prev=reach,uncovered,len(walls),area
   if t==300: final=reach,area,len(uncovered)
  same=hashlib.sha256(actions).hexdigest()==known['actions_sha256']
  out.append(dict(case=f.name,score=score,exact=same,final=final,below300=first,events=events))
  print(f.name,'exact=',same,'final=',final,'below300=',first,flush=True)
Path('ahc008_v24_m5_trace_audit/v24-audit.json').write_text(json.dumps(out,indent=2))
