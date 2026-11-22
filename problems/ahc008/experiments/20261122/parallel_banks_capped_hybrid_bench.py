"""Official paired baseline vs sparse checkerboard one-ended banks and exact geometry replay."""
import argparse, csv, hashlib, re, sys, tempfile
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL, B, idx, flood, wave
from parallel_banks_wide_bench import count

def typed(hist, kinds, geometry, release, active):
    tubes=[]
    for line in geometry:
        b,r,l,g,k,rr,*rest=map(int,line.split())
        floor=sum(B[q] for q in rest[:4]); brace=sum(B[q] for q in rest[4:])
        ix=(r-3)//2
        cap=-1
        if active and (ix+b)%3==1:
            cap,op,stand=(g,k,rr) if (ix//3+b)%2==0 else (k,g,l)
        else:
            op=stand=-1
        tubes.append((floor,brace,g,k,cap,op,stand))
    pairs=set();singles=set();elig=set();staff=set()
    caps={};has={};obs=staff_w=0
    for t in range(release):
        ps,hs,ws=hist[t]; pn,hn,wn=hist[t+1]
        wall=sum(B[idx(w)] for w in ws)
        after=sum(B[idx(w)] for w in wn)
        add=after^wall
        people=sum({B[idx(p)] for p in hs}); animals=sum({B[idx(p)] for p in ps})
        bad=people|animals|wave(animals)
        for ti,(body,brace,g,k,cap,op,stand) in enumerate(tubes):
            both=B[g]|B[k]
            if brace&wall!=brace:continue
            if cap>=0 and add&B[cap] and not ((wall|add)&B[op]):
                assert not (body&(people|animals)) and not (B[cap]&bad)
                assert ti not in caps
                caps[ti]=t
            if not body&animals or body&people:continue
            inside={j for j,p in enumerate(ps) if B[idx(p)]&body}
            two=not (wall&both) and add&both==both
            one=cap>=0 and bool(wall&B[cap]) and not wall&B[op]
            if one and not bad&B[op]:
                assert ti in caps
                obs+=1;elig|=inside
                if people&B[stand]:
                    staff_w+=1;staff|=inside
                if add&B[op]:
                    assert people&B[stand]
                    assert not (inside&(pairs|singles))
                    singles|=inside
                else:one=False
            else:one=False
            if two:
                assert not (inside&(pairs|singles))
                pairs|=inside
            if two or one:
                y=ALL^after
                assert flood(body&-body,y)==body
                assert not body&sum({B[idx(p)] for p in hn})
                assert all(body&B[idx(pn[i])] for i in inside)
    row=dict(caps_prepared=len(caps),cap_first=min(caps.values(),default=-1),
             caps_at50=sum(t<50 for t in caps.values()),caps_at101=sum(t<101 for t in caps.values()),
             single_legal_windows=obs,single_staffed_windows=staff_w,
             single_unique_eligible=len(elig),single_unique_staffed=len(staff),
             direct_singles=len(singles),direct_total=len(pairs|singles),
             direct_ids='+'.join(map(str,sorted(pairs|singles))),
             single_ids='+'.join(map(str,sorted(singles))))
    for s in range(1,6):
        row[f'single_species_{s}']=sum(kinds[p]==s for p in singles)
    return row,pairs|singles

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
        for model,mode in (('narrow',0),('hybrid',1)):
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
                else:assert meta.splitlines()[1:]==state[0].splitlines()[1:]
                row.update(count(hist,kinds,meta))
                split, animals=typed(hist,kinds,rest,release,m>=7 and mode==1)
                assert split['direct_total']==row['bank_capture4']+split['direct_singles']
                row.update(split)
                if not mode:before=animals;row.update(extra_different_ids=0,missed_different_ids=0)
                else:row.update(extra_different_ids=len(animals-before),missed_different_ids=len(before-animals))
                out.append(row)
        with a.output.open('w') as z:
            w=csv.DictWriter(z,dict.fromkeys(k for r in out for k in r),lineterminator='\n')
            w.writeheader();w.writerows(out)
        print(f.name,flush=True)
if __name__=='__main__':main()
