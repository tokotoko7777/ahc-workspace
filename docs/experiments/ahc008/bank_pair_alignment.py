"""Trace diagnosis only: how far are adjacent-aisle guards at open mouth windows?"""
import argparse,csv,hashlib,sys,tempfile
from pathlib import Path
from collections import Counter,defaultdict

a=argparse.ArgumentParser()
for k in ('experiments','harness','tester','solver','inputs','ledger','out'):a.add_argument(k,type=Path)
a=a.parse_args()
sys.path[:0]=[str(a.experiments.resolve()),str(a.harness.resolve())]
from benchmark_cases import run
from benchmark import replay
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,flood,wave
base={r['case']:r for r in csv.DictReader(a.ledger.open()) if r['model']=='rescue160'}
histograms=Counter(); unresolved=defaultdict(set)
rows=[]
for q in sorted(a.inputs.glob('*.txt')):
  c=q.read_bytes(); rec=base[q.name]
  assert hashlib.sha256(c).hexdigest()==rec['input_sha256']
  with tempfile.TemporaryDirectory() as top:
    meta=Path(top)/'bank'; log=Path(top)/'trace'; af=Path(top)/'actions'
    actions,stderr,rc,_=run([a.tester.resolve(),'env',f'AHC008_BANK_LOG={meta}',a.solver.resolve(),'5','1'],c,4)
    assert rc==0 and hashlib.sha256(actions).hexdigest()==rec['actions_sha256']
    af.write_bytes(actions)
    assert run([a.tester.resolve(),sys.executable,a.harness.resolve()/'benchmark_cases.py','--playback',af,log],c,9)[:3]==(actions,stderr,0)
    commands=log.read_text().splitlines()
    assert replay(c.decode(),commands,actions.decode().splitlines(),detailed=True)[0]==int(rec['score'])
    _,hist,_=observed(c,commands)
    lines=meta.read_text().splitlines()
  rot,shift,_=map(int,lines[0].split())
  records=[]
  for l in lines[1:]:
    j,r,ls,g,k,rs,*ps=map(int,l.split())
    records.append((j,r,(ls,rs),B[g]|B[k],sum(B[z] for z in ps[:4]),sum(B[z] for z in ps[4:])))
  ps,hs,ws=hist[-1]
  keep=flood(sum({B[idx(h)] for h in hs}),ALL^sum(B[idx(p)] for p in ws))
  final={i for i,p in enumerate(ps) if B[idx(p)]&keep}
  cut=int(rec['cutoff'])
  cs=Counter(); seen=defaultdict(set)
  for t,(ps,hs,ws) in enumerate(hist[101:cut],101):
    walls=sum(B[idx(w)] for w in ws); people=sum({B[idx(h)] for h in hs}); pets=sum({B[idx(p)] for p in ps})
    forbid=people|pets|wave(pets)
    rows_a=[[] for j in range(4)]
    for x,y in hs:
      v,w=(x,y) if rot else (y,x)
      if v in [shift+7*j for j in range(4)]:
        rows_a[(v-shift)//7].append(w)
    for j,r,guard,doors,pen,brace in records:
      if doors&(walls|forbid) or brace&walls!=brace or not pen&pets or pen&people:continue
      ids={i for i,p in enumerate(ps) if pen&B[idx(p)]}
      pair=rows_a[j],rows_a[j+1]
      if all(people&B[g] for g in guard):
        tag='staffed'
      elif not all(pair):
        tag='missing_lane'
      elif set(pair[0])&set(pair[1]):
        tag='aligned_somewhere'
      elif max(min(abs(y-r) for y in p) for p in pair)<=2:
        tag='unaligned_near'
      else:
        tag='unaligned_far'
      cs[tag]+=1; seen[tag]|=ids&final
  histograms.update(cs)
  for k,v in seen.items():unresolved[k].update((q.name,i) for i in v)
  rows.append((q.name,*[v for k in ('staffed','missing_lane','aligned_somewhere','unaligned_near','unaligned_far') for v in (cs[k],len(seen[k]))]))
with a.out.open('w') as f:
  w=csv.writer(f);w.writerow(['case']+[k+s for k in ('staffed','missing_lane','aligned_somewhere','unaligned_near','unaligned_far') for s in ('_windows','_finally_missed_pets')]);w.writerows(rows)
print(histograms,{k:len(v) for k,v in unresolved.items()})
