"""Run sequential official v3 pairs and independently verify first global-clear halt."""
import argparse
import csv
import hashlib
import re
import sys
import tempfile
from pathlib import Path

def make(tester,binary,case,run,replay,make_history,playback,tmp):
    tape=tmp/'tape'
    log=tmp/'clear'
    command=[tester,'env','AHC008_GLOBAL_CLEAR_LOG='+str(log),binary]
    output,err,rc,t=run(command,case,3.0)
    match=re.fullmatch(rb'Score = (\d+)\s*',err)
    assert rc==0 and match,(err,rc)
    tape.write_bytes(output)
    trace=tmp/'trace'
    ack,newerr,rc2,_=run([tester,sys.executable,playback,'--playback',tape,trace],case,10.)
    assert (ack,newerr,rc2)==(output,err,0)
    actions=output.decode().splitlines()
    lines=trace.read_text().splitlines()
    score,*_=replay(case.decode(),lines,actions,detailed=True)
    assert len(actions)==300 and score==int(match[1])
    _,history,_=make_history(case,lines)
    assert len(history)==301
    return score,t,actions,output,history,int(log.read_text()) if log.exists() else -1

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','base','candidate','cases','out'):
        p.add_argument(n,type=Path)
    p.add_argument('--n',type=int,default=30)
    z=p.parse_args()
    sys.path.insert(0,str(z.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    from guarded_entry_trace import observed
    from stochastic_lock_audit import ALL,B,idx,flood
    cases=sorted(z.cases.glob('*.txt'))[:z.n]
    assert len(cases)==z.n
    td=[q.resolve() for q in (z.tester,z.base,z.candidate)]
    out=[]
    for file in cases:
        case=file.read_bytes()
        arms=[]
        for binary in td[1:]:
            with tempfile.TemporaryDirectory() as t:
                arms.append(make(td[0],binary,case,run,replay,observed,z.harness.resolve()/'benchmark_cases.py',Path(t)))
        a,b=arms
        earliest=-1
        for k,(ps,hs,ws) in enumerate(a[4][:-1]):
            free=ALL^sum(B[idx(w)] for w in ws)
            people=flood(sum({B[idx(h)] for h in hs}),free)
            if not any(people&B[idx(q)] for q in ps):
                earliest=k;break
        assert b[5]==earliest,(file,earliest,b[5])
        if earliest<0:
            assert a[2]==b[2],file
        else:
            assert a[2][:earliest]==b[2][:earliest],file
            assert all(set(cmd)=={'.'} for cmd in b[2][earliest:]),file
        assert b[0]>=a[0],(file,a[0],b[0])
        out.append(dict(case=file.name,input_sha256=hashlib.sha256(case).hexdigest(),
            stop_turn=earliest,baseline=a[0],candidate=b[0],delta=b[0]-a[0],
            baseline_wall_seconds=a[1],candidate_wall_seconds=b[1],
            baseline_actions_sha256=hashlib.sha256(a[3]).hexdigest(),
            candidate_actions_sha256=hashlib.sha256(b[3]).hexdigest(),
            baseline_walls_after_stop=sum(c in 'udlr' for ln in a[2][earliest:] for c in ln) if earliest>=0 else 0))
        with z.out.open('w') as f:
            w=csv.DictWriter(f,fieldnames=list(out[0]),lineterminator='\n');w.writeheader();w.writerows(out)
        print(file.name,flush=True)
if __name__=='__main__':main()
