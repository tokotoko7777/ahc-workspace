"""Observational audit of one frozen stall criterion in AHC008 bank rescue."""
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
rows=[]
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
    _,hist,_=observed(cb,commands)
    residual=[]
    for ps,hs,walls in hist[:161]:
        open_=ALL^sum(B[idx(w)] for w in walls)
        ours=flood(sum({B[idx(h)] for h in hs}),open_)
        residual.append(sum(bool(ours&B[idx(x)]) for x in ps))
    # One candidate rule; never used to make the logged baseline's choices.
    stamp=next((t for t in range(110,151) if residual[t]>=7 and residual[t]==residual[t-25]),-1)
    rows.append((q.name,stamp,residual[stamp] if stamp>=0 else '',residual[160],
                 residual[stamp]-residual[160] if stamp>=0 else '',
                 int(r0['final_residual_pets']),score))
with a.output.open('w') as f:
    w=csv.writer(f);w.writerow(('case','stall_turn','reachable_on_stall','reachable160','captured_after_stall_before_160','final_residual','official_score'));w.writerows(rows)
sel=[r for r in rows if r[1]>=0]
print('confirmed',len(rows),'stall candidates',len(sel),'pre160 more captures',sum(r[4] for r in sel),'final all clear',sum(r[5]==0 for r in sel))
