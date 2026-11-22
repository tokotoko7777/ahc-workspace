"""Read-only census of honest closure opportunities along frozen v24 replay."""
import csv, hashlib, json, subprocess, sys, tempfile
from collections import Counter
from pathlib import Path
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
nb=[[i+j for j in (-30,30,-1,1) if 0<=i+j<900 and (abs(j)==30 or (i+j)//30==i//30)] for i in range(900)]
def close_reach(hs,wall):
 d=[{} for _ in hs]
 for i,h in enumerate(hs):
  q=d[i];q[h]=0
  for x in nb[h]:
   if x not in wall:q[x]=1
  for x in list(q):
   if q[x]!=1:continue
   for y in nb[x]:
    if y not in wall and y not in q:q[y]=2
 return d
out=[]
for f in sorted((p/'m5-dev30/in').glob('*.txt')):
 raw=f.read_bytes();known=ref[f.name]
 assert hashlib.sha256(raw).hexdigest()==known['input_sha256']
 with tempfile.TemporaryDirectory() as d:
  pth=Path(d);a=pth/'a'; z=pth/'trace'; g=pth/'geo'
  actions,err,rc,_=run([str(T),'env','AHC008_BANK_LOG='+str(g),str(EXE)],raw,4.5)
  assert rc==0; a.write_bytes(actions)
  assert hashlib.sha256(actions).hexdigest()==known['actions_sha256']
  b,newerr,newrc,_=run([str(T),sys.executable,H/'benchmark_cases.py','--playback',a,z],raw,12.)
  assert (b,newerr,newrc)==(actions,err,0)
  moves=z.read_text().splitlines()
  score,*_=replay(raw.decode(),moves,actions.decode().splitlines(),detailed=True)
  assert err.decode().strip()==f'Score = {score}'
  kinds,hist,_=observed(raw,moves)
  tubes=[]
  for x in g.read_text().splitlines()[1:]:
   numbers=list(map(int,x.split()))
   assert len(numbers)==18, numbers
   l,ga,gb,h=numbers[2:6]; inner=numbers[6:10];side=numbers[10:]
   tubes.append((l,ga,gb,h,inner,side))
 final_pets,final_hs,final_wall=hist[-1]
 root=flood(sum(B[i] for i in {idx(q) for q in final_hs}),ALL^sum(B[idx(q)] for q in final_wall))
 leftover={i for i,p in enumerate(final_pets) if root&B[idx(p)]}
 metrics={i:{'kind':kinds[i],'leftover':int(i in leftover),'prepared_body':0,'legal_hypothetical':0,'both_staffed':0,'pair_reach2':0,'unique_tubes':set()} for i in range(len(kinds))}
 release=int(known['reported_release'])
 for t,(positions,human,braces) in enumerate(hist):
  if t<max(160,release) or t>=275:continue
  ps=list(map(idx,positions));hs=list(map(idx,human));wall=set(map(idx,braces))
  if not ps:continue
  blocked=set(ps)|set(hs)
  for a in ps:blocked.update(nb[a])
  for j,(l,a,b,r,inside,outside) in enumerate(tubes):
   if a in wall or b in wall or not all(q in wall for q in outside):continue
   q=[i for i,p in enumerate(ps) if p in inside]
   if not q:continue
   for i in q:metrics[i]['prepared_body']+=1
   if a in blocked or b in blocked or any(h in inside for h in hs):continue
   d=close_reach(hs,wall|{a,b})
   pair=min((max(x.get(l,3),y.get(r,3)) for i,x in enumerate(d) for j,y in enumerate(d) if i!=j),default=3)
   for i in q:
    item=metrics[i]
    item['legal_hypothetical']+=1
    item['both_staffed']+=pair==0
    item['pair_reach2']+=pair<=2
    item['unique_tubes'].add(j)
 for q in metrics.values():q['unique_tubes']=sorted(q['unique_tubes'])
 out.append(dict(case=f.name,release=release,metrics=metrics))
 print(f.name,flush=True)
p=Path(__file__).resolve().parent
(p/'result.json').write_text(json.dumps(out)+'\n')
z=[q for r in out for q in r['metrics'].values() if q['leftover']]
print(len(z),dict(Counter(q['kind'] for q in z)))
for kind in range(1,6):
 q=[r for r in z if r['kind']==kind]
 print(kind,len(q),{k:sum(bool(r[k]) for r in q) for k in ['prepared_body','legal_hypothetical','both_staffed','pair_reach2']})
print('all',len(z),{k:sum(bool(r[k]) for r in z) for k in ['prepared_body','legal_hypothetical','both_staffed','pair_reach2']})
