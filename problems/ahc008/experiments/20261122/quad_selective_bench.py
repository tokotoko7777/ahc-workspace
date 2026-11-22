"""One frozen, visible forecast filter on prior dev30; exact official action matching."""
import argparse,csv,hashlib,re,sys,tempfile
from pathlib import Path
from guarded_entry_trace import observed
from quad_deferred_bench import stats
ARMS=(('control',0,0),('full-deferred',2,0),('selective-deferred',2,1))
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ['harness','tester','baseline','pilot','cases','out']:p.add_argument(n,type=Path)
    p.add_argument('--case-count',type=int,default=30)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester=a.tester.resolve();playback=a.harness.resolve()/'benchmark_cases.py'
    rows=[]
    files=sorted(a.cases.glob('*.txt'))[:a.case_count]
    for f in files:
        case=f.read_bytes()
        config=None
        arm_outputs={}
        for tag,k,selective in ARMS:
            with tempfile.TemporaryDirectory() as td:
                probe=Path(td)/'quad'
                command=[tester,a.baseline.resolve()] if k==0 else [tester,'env',f'AHC008_QUAD_LOG={probe}',a.pilot.resolve(),str(k),'27','1',str(selective)]
                stdout,stderr,code,time=run(command,case,3.)
                match=re.fullmatch(rb'Score = (\d+)\s*',stderr)
                assert code==0 and match,(f.name,tag,stderr)
                act=Path(td)/'actions';act.write_bytes(stdout);trace=Path(td)/'trace'
                stdout2,stderr2,c2,_=run([tester,sys.executable,playback,'--playback',act,trace],case,10.)
                assert (stdout2,stderr2,c2)==(stdout,stderr,0)
                data=trace.read_text().splitlines()
                score,*_=replay(case.decode(),data,stdout.decode().splitlines(),detailed=True)
                assert int(match[1])==score
                kinds,hist,_=observed(case,data)
                row={'case':f.name,'model':tag,'score':score,'wall_s':round(time,6),
                     'input_sha256':hashlib.sha256(case).hexdigest(),'actions_sha256':hashlib.sha256(stdout).hexdigest()}
                side=probe.read_text() if k else None
                if k:
                    current=side.splitlines()[0]
                    if config is None:config=current
                    assert config==current,(f.name,tag)
                add={}
                if selective:
                    m,measure,*log=side.splitlines()
                    _,eta,prob,active=measure.split()
                    add=dict(forecast_eta=int(eta),forecast_prob=float(prob),activated=int(active))
                    ref=arm_outputs['full-deferred' if int(active) else 'control']
                    assert (stdout,stderr,code)==ref,(f.name,active)
                    side='\n'.join([m,*log]) if int(active) else None
                row.update(stats(hist,kinds,side))
                row.update(add)
                arm_outputs[tag]=(stdout,stderr,code)
                rows.append(row)
        fields=list(dict.fromkeys(k for row in rows for k in row))
        with a.out.open('w') as file:
            writer=csv.DictWriter(file,fields,lineterminator='\n');writer.writeheader();writer.writerows(rows)
        print(f.name,flush=True)
if __name__=='__main__':main()
