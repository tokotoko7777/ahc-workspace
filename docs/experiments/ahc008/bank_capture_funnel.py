"""Attribute each animal's first isolation to prepared banks or salvage."""
import argparse,csv,hashlib,re,sys,tempfile
from pathlib import Path

p=argparse.ArgumentParser()
for name in ('experiments','harness','tester','solver','inputs','ledger','output'):p.add_argument(name,type=Path)
a=p.parse_args();sys.path[:0]=[str(a.experiments.resolve()),str(a.harness.resolve())]
from benchmark_cases import run
from benchmark import replay
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,flood
old={r['case']:r for r in csv.DictReader(a.ledger.open()) if r['model']=='rescue160'}
rows=[]; ready=[]
for q in sorted(a.inputs.glob('*.txt')):
    cb=q.read_bytes();r0=old[q.name]
    assert hashlib.sha256(cb).hexdigest()==r0['input_sha256']
    act,err,status,_=run([a.tester.resolve(),a.solver.resolve(),'5','1'],cb,4)
    assert status==0 and hashlib.sha256(act).hexdigest()==r0['actions_sha256']
    with tempfile.TemporaryDirectory() as temp:
        pa=Path(temp)/'actions';pa.write_bytes(act)
        tr=Path(temp)/'trace'
        ck,er,rc,_=run([a.tester.resolve(),sys.executable,a.harness.resolve()/'benchmark_cases.py','--playback',pa,tr],cb,9)
        assert (ck,er,rc)==(act,err,0)
        commands=tr.read_text().splitlines()
    score,*_=replay(cb.decode(),commands,act.decode().splitlines(),detailed=True)
    assert score==int(r0['score'])

    kinds,hist,_=observed(cb,commands)
    cutoff=int(r0['cutoff'])
    first={}
    for t,(ps,hs,walls) in enumerate(hist):
        op=ALL^sum(B[idx(w)] for w in walls)
        people=flood(sum({B[idx(h)] for h in hs}),op)
        for i,p in enumerate(ps):
            if people&B[idx(p)]:
                assert i not in first
                continue
            if i not in first:
                first[i]=(t,flood(B[idx(p)],op).bit_count())
    # Pre-registered diagnosis: currently at most one directed type
    # (dog/cat) and at least five passive animals at t130.
    # Continue applying normal saved rescue; this does not test causality.
    left=[i for i in range(len(kinds)) if i not in first or first[i][0]>130]
    passive=sum(kinds[i]<=3 for i in left); directed=len(left)-passive
    if passive>=5 and directed<=1:
        ready.append((q.name, passive, directed, sum(130<first[i][0]<=160 for i in left if i in first),
                      int(r0['final_residual_pets']), score))
    for i,k in enumerate(kinds):
        t,area=first.get(i,(-1,0))
        rows.append((q.name,i,k,cutoff,t,area,
                     'bank' if 0<=t<=cutoff else 'fallback' if t>=0 else 'unisolated'))
with a.output.open('w') as f:
    w=csv.writer(f);w.writerow(('case','pet_index','species','rescue_cutoff','first_isolated_turn','first_isolated_area','phase'));w.writerows(rows)
from collections import Counter
out=Counter((r[2],r[-1]) for r in rows)
print('kind,bank,fallback,unisolated')
for k in range(1,6):
    print(k,*[out[k,s] for s in ('bank','fallback','unisolated')],sep=',')
print('initial and final reachable',len(rows),sum(r[-1]=='unisolated' for r in rows))

print('compositional trigger',len(ready),'bank catches after130 through160',sum(r[3] for r in ready))
for r in ready: print(*r,sep=',')
