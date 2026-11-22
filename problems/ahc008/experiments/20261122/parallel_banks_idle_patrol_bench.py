"""Paired truly idle bank patrol x frozen rescue160, official replay."""
import argparse, csv, hashlib, re, sys, tempfile
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL, B, idx, flood
from parallel_banks_wide_bench import count

def closes(hist,kinds,release,actions):
    """Use pre-response positions and post-wall grid; count actual first disconnect."""
    seen=set()
    results=[]
    for t in range(release,300):
        if not any(c.islower() for c in actions[t]): continue
        ps, hs, ws=hist[t]
        _, hp, wp=hist[t+1]
        newclosed=ALL^sum(B[idx(w)] for w in wp)
        if ws==wp: continue
        before=flood(sum({B[idx(h)] for h in hs}),ALL^sum(B[idx(w)] for w in ws))
        after=flood(sum({B[idx(h)] for h in hp}),newclosed)
        for i,p in enumerate(ps):
            bp=B[idx(p)]
            if i in seen or not (before&bp) or after&bp:continue
            seen.add(i); pen=flood(bp,newclosed)
            size=pen.bit_count()
            if not (4<=size<=48):continue
            points=[divmod(j,30) for j in range(900) if pen&B[j]]
            ys,xs=zip(*points)
            rect=(max(ys)-min(ys)+1)*(max(xs)-min(xs)+1)==size
            results.append((t,i,kinds[i],size,int(not rect)))
    return results

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','pilot','cases','output'):p.add_argument(n,type=Path)
    p.add_argument('--case-count',type=int,default=30)
    p.add_argument('--frozen-control-csv',type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    frozen={}
    if a.frozen_control_csv:
        with a.frozen_control_csv.open() as f:
            frozen={r['case']:r for r in csv.DictReader(f) if r['model']=='rescue160'}
    out=[]; tester=a.tester.resolve()
    for f in sorted(a.cases.glob('*.txt'))[:a.case_count]:
        case=f.read_bytes()
        ss=case.splitlines();m=int(ss[int(ss[0])+1]);state=None
        for model,mode in (('rescue',0),('patrol',1)):
            with tempfile.TemporaryDirectory() as td:
                q=Path(td)/'meta'; stage=Path(td)/'rescue'; diagnostic=Path(td)/'flow'
                command=[tester,'env',f'AHC008_BANK_LOG={q}',f'AHC008_RESCUE_LOG={stage}',f'AHC008_PATROL_LOG={diagnostic}']
                command += [a.pilot.resolve(),'5','1',str(mode)]
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
                w=act.splitlines()
                pair=(rot,shift,rest,w[:101])
                if state is None:
                    if frozen:
                        prior=frozen[f.name]
                        assert int(prior['score'])==score and prior['actions_sha256']==row['actions_sha256'], f.name
                    state=pair; before=w
                else:
                    assert pair==state,(f.name,'bank history differs')
                row['first_diff']=next((i for i,(u,v) in enumerate(zip(w,before)) if u!=v),-1)
                flow=map(int,diagnostic.read_text().split())
                row.update(zip(('patrol_eligible','patrol_applied','patrol_actual','patrol_turns'),flow))
                close=closes(hist,kinds,release,act.decode().splitlines())
                row.update(compact_cuts=len(close),irregular_cuts=sum(z[-1] for z in close),
                           compact_witnesses=';'.join(' '.join(map(str,z)) for z in close))
                row.update(count(hist,kinds,meta));out.append(row)
        with a.output.open('w') as z:
            w=csv.DictWriter(z,dict.fromkeys(k for r in out for k in r),lineterminator='\n')
            w.writeheader();w.writerows(out)
        print(f.name,flush=True)
if __name__=='__main__':main()
