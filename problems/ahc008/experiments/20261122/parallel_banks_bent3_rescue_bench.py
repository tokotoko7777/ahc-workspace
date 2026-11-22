"""Official paired narrow rescue vs irregular bent high-worker rescue and exact geometry replay."""
import argparse, csv, hashlib, re, sys, tempfile
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL, B, idx, flood
from parallel_banks_wide_bench import count

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','baseline','pilot','cases','output'):p.add_argument(n,type=Path)
    p.add_argument('--case-count',type=int,default=30)
    p.add_argument('--low-workers',action='store_true')
    p.add_argument('--mixed',action='store_true')
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    out=[]; tester=a.tester.resolve()
    for f in sorted(a.cases.glob('*.txt'))[:a.case_count]:
        case=f.read_bytes()
        ss=case.splitlines();m=int(ss[int(ss[0])+1]);state=None
        assert a.mixed or (m<7)==a.low_workers
        for model,mode in (('narrow',0),('bent3_rescue',1)):
            with tempfile.TemporaryDirectory() as td:
                q=Path(td)/'meta'; stage=Path(td)/'rescue'
                command=[tester,'env',f'AHC008_BANK_LOG={q}',f'AHC008_RESCUE_LOG={stage}']
                command += [a.pilot.resolve(),'5','1','1',str(mode)] if mode else [a.baseline.resolve()]
                act,err,rc,t=run(command,case,3.)
                match=re.fullmatch(rb'Score = (\d+)\s*',err)
                assert rc==0 and match,(f.name,model,err)
                actfile=Path(td)/'a';actfile.write_bytes(act);tr=Path(td)/'trace'
                ack,err2,rc2,_=run([tester,sys.executable,a.harness.resolve()/'benchmark_cases.py','--playback',actfile,tr],case,10.)
                assert (ack,err2,rc2)==(act,err,0)
                lines=tr.read_text().splitlines()
                score,*_=replay(case.decode(),lines,act.decode().splitlines(),detailed=True)
                assert score==int(match[1])
                kinds,hist,_=observed(case,lines)
                row=dict(case=f.name,model=model,workers=m,score=score,time=t,
                         input_sha256=hashlib.sha256(case).hexdigest(),actions_sha256=hashlib.sha256(act).hexdigest())
                meta=q.read_text()
                n160,release=map(int,stage.read_text().split())
                assert release==(160 if n160>4 else 225)
                stem,*rest=meta.splitlines(); rot,shift,_=map(int,stem.split())
                meta='\n'.join([f'{rot} {shift} {release}',*rest])
                animals,people,walls=hist[160]
                grid=ALL^sum(B[idx(w)] for w in walls)
                accessible=flood(B[idx(people[0])],grid)
                assert all(B[idx(h)]&accessible for h in people)
                assert sum(bool(B[idx(p)]&accessible) for p in animals)==n160
                row.update(residual160=n160,release=release)
                animals,people,walls=hist[-1]
                grid=ALL^sum(B[idx(w)] for w in walls)
                pets=sum({B[idx(p)] for p in animals})
                safe=0; cleared=0; mean=0
                for h in people:
                    comp=flood(B[idx(h)],grid)
                    if not comp&pets:
                        safe|=comp; cleared+=1; mean+=comp.bit_count()
                row.update(safe_union=safe.bit_count(),cleared_humans=cleared,mean_safe_area=mean/len(people))
                pair=meta,act
                if state is None:state=pair
                elif m<7:assert state==pair,(f.name,'low-worker fallback differs')
                else:
                    assert len(meta.splitlines())==28 and len(state[0].splitlines())==37
                    placements=[list(map(int,z.split())) for z in meta.splitlines()[1:]]
                    assert {z[0] for z in placements}=={0,1,2}
                    assert len({p for z in placements for p in z[10:]})==138
                    for z in placements:
                        pp=[divmod(q,30) for q in z[6:10]]
                        assert len(set(r for r,c in pp))*len(set(c for r,c in pp))==6
                        x,y=divmod(z[2],30); xx,yy=divmod(z[5],30)
                        assert sorted([abs(xx-x),abs(yy-y)])==[1,6]
                row.update(count(hist,kinds,meta));out.append(row)
        with a.output.open('w') as z:
            w=csv.DictWriter(z,dict.fromkeys(k for r in out for k in r),lineterminator='\n')
            w.writeheader();w.writerows(out)
        print(f.name,flush=True)
if __name__=='__main__':main()
