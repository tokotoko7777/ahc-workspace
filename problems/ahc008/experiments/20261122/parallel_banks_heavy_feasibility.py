"""Read-only viability of already-enumerated two-plus-animal late plans."""
import argparse,csv,hashlib,sys,tempfile
from pathlib import Path

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','instrumented','cases','control','output'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with a.control.open() as f:
        target=[r for r in csv.DictReader(f) if r['model']=='narrow' and int(r['residual160'])>=11]
    out=[]; rowcases=[]
    for r in target:
        case=(a.cases/r['case']).read_bytes()
        with tempfile.TemporaryDirectory() as d:
            b=Path(d)/'audit'; actfile=Path(d)/'actions'; trace=Path(d)/'trace'
            cmd=[a.tester.resolve(),'env',f'AHC008_HEAVY_AUDIT={b}',a.instrumented.resolve()]
            acts,err,rc,_=run(cmd,case,3.)
            assert rc==0 and err.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(acts).hexdigest()==r['actions_sha256']
            actfile.write_bytes(acts)
            play,err2,rc2,_=run([a.tester.resolve(),sys.executable,a.harness.resolve()/'benchmark_cases.py',
                                '--playback',actfile,trace],case,10.)
            assert (play,err2,rc2)==(acts,err,0)
            assert int(r['score'])==replay(case.decode(),trace.read_text().splitlines(),acts.decode().splitlines(),detailed=True)[0]
            lines=b.read_text().splitlines() if b.exists() else []
            rowcases.append((r['case'],len(lines)))
            for line in lines:
                row=dict(zip(('turn','residual160','nominal','exact','assignable','max_pets',
                              'assignable_static','assignable_flow','original_pets','original_jobs','best_yield'),
                             map(int,line.split())))
                assert row['residual160']==int(r['residual160'])
                row['case']=r['case']
                row['novel_better']=int(row['best_yield']>row['original_pets']*1000//max(1,row['original_jobs']))
                out.append(row)
        print(r['case'],len(lines))
    with a.output.open('w') as f:
        writer=csv.DictWriter(f,list(out[0]),lineterminator='\n');writer.writeheader();writer.writerows(out)
    ready=[r for r in out if r['assignable']]
    novel=[(r['case'],r['turn']) for r in ready if r['novel_better'] and r['turn']<=294]
    print('verified',rowcases,'calls',len(out),'ready',len(ready),'novel_unique_turns',len(set(novel)),'novel_cases',len({q[0] for q in novel}))

if __name__=='__main__':main()
