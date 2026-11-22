"""Official paired narrow default vs x7-20-13 perpendicular banks and exact geometry replay."""
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
        for model,mode in (('narrow',0),('orthogonal',1)):
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
                    assert len(meta.splitlines())==35 and len(state[0].splitlines())==37
                    entries=[list(map(int,z.split())) for z in rest]
                    assert [sum(z[0]==i for z in entries) for i in range(4)]==[7,7,7,13]
                    assert len({p for z in entries for p in z[10:]})==152
                    def coord(p):
                        xy=divmod(p,30)
                        return xy[::-1] if rot else xy
                    for j,t,l,g,k,h,*more in entries:
                        p,q=coord(l),coord(h)
                        if j==3:
                            assert p==(20,t) and q==(27,t) and t in range(3,28,2)
                            assert set(map(coord,more[:4]))=={(v,t) for v in range(22,26)}
                        else:
                            assert p==(t,4+7*j) and q==(t,11+7*j) and t in range(2,15,2)
                            assert set(map(coord,more[:4]))=={(t,v) for v in range(6+7*j,10+7*j)}
                row.update(count(hist,kinds,meta));out.append(row)
        with a.output.open('w') as z:
            w=csv.DictWriter(z,dict.fromkeys(k for r in out for k in r),lineterminator='\n')
            w.writeheader();w.writerows(out)
        print(f.name,flush=True)
if __name__=='__main__':main()
