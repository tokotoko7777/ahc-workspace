"""Audit reachable/local-wall matching without changing actions."""
import argparse,csv,hashlib,json,sys,tempfile
from pathlib import Path

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','instrumented','cases','control','output','json'):
        p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with a.control.open() as f:
        ref=[r for r in csv.DictReader(f) if r['model']=='narrow']
    out=[]
    for r in ref:
        path=a.cases/r['case']; case=path.read_bytes()
        with tempfile.TemporaryDirectory() as d:
            b=Path(d)/'audit'
            cmd=[a.tester.resolve(),'env',f'AHC008_DISPATCH_AUDIT={b}',a.instrumented.resolve()]
            acts,err,rc,_=run(cmd,case,3.)
            assert rc==0 and err.strip()==('Score = '+r['score']).encode(), (r['case'],err)
            assert hashlib.sha256(acts).hexdigest()==r['actions_sha256'],r['case']
            af=Path(d)/'actions';trace=Path(d)/'trace';af.write_bytes(acts)
            play,err2,rc2,_=run([a.tester.resolve(),sys.executable,a.harness.resolve()/'benchmark_cases.py',
                                '--playback',af,trace],case,10.)
            assert (play,err2,rc2)==(acts,err,0)
            assert int(r['score'])==replay(case.decode(),trace.read_text().splitlines(),acts.decode().splitlines(),detailed=True)[0]
            for line in b.read_text().splitlines() if b.exists() else []:
                row=dict(zip(('turn','target','phase','jobs','human_inside','greedy_now','max_now','safe_extra',
                              'greedy_travel','max_travel'),map(int,line.split())))
                row['case']=r['case']
                assert row['turn']>=int(r['release'])
                assert row['max_now']>=row['greedy_now'] and row['max_travel']>=row['greedy_travel']
                out.append(row)
    with a.output.open('w') as f:
        wr=csv.DictWriter(f,list(out[0]),lineterminator='\n');wr.writeheader();wr.writerows(out)
    def rec(a,b,p):
        rows=[r for r in out if p(r)]
        return dict(calls=len(rows),turns=len({(r['case'],r['turn']) for r in rows}),
                    targets=len({(r['case'],r['target'],r['phase']) for r in rows}),
                    cases=len({r['case'] for r in rows}),
                    sum_extra=sum(r[a]-r[b] for r in rows))
    s=dict(verified_cases=len(ref),total_calls=len(out),
           raw_immediate=rec('max_now','greedy_now',lambda r:r['max_now']>r['greedy_now']),
           safe_immediate=rec('max_now','greedy_now',lambda r:r['safe_extra']),
           conditional_travel=rec('max_travel','greedy_travel',lambda r:r['max_travel']>r['greedy_travel']))
    a.json.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))

if __name__=='__main__':main()
