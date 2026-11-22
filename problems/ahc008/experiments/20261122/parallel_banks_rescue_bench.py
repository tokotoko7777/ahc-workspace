"""Live parallel exact-four tube banks: official replay and per-window accounting."""
import argparse,csv,hashlib,sys,tempfile,statistics,re
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ADJ,ALL,B,idx,wave,flood
from quad_live_bench import all_pen_counts

def count(hist,kinds,meta):
    final,res=all_pen_counts(*hist[-1],kinds)
    row={f'final4_{i}':final[i] for i in range(1,6)}
    ps,hs,ws=hist[-1]
    opened=ALL^sum(B[idx(w)] for w in ws)
    area=[flood(B[idx(h)],opened).bit_count() for h in hs]
    safe=0
    pbits=sum({B[idx(p)] for p in ps})
    for h in hs:
        comp=flood(B[idx(h)],opened)
        if not comp&pbits:safe|=comp
    row.update(final_safe_union=safe.bit_count(),final_residual_pets=res,final_mean_human_area=sum(area)/len(area),
               final_min_human_area=min(area),final_human_union=flood(sum({B[idx(h)] for h in hs}),opened).bit_count())
    if not meta:return row
    head,*lines=meta.splitlines()
    rot,shift,stop=map(int,head.split())
    placements=[list(map(int,l.split())) for l in lines]
    tubes=[]
    for b,r,ls,g,k,rs,*z in placements:
        inner=sum(B[q] for q in z[:4]);side=sum(B[q] for q in z[4:])
        assert wave(inner)&~inner==side|B[g]|B[k]
        tubes.append((ls,g,k,rs,inner,side))
    wire=set(q for tube in tubes for q in range(900) if tube[5]&B[q])
    first={};captured=set();legal_first={}; delays=[]
    blocks=[0]*6
    late=0
    ready5=-1
    crawled= sum(B[(r*30+c if not rot else c*30+r)] for r in range(30)
                 for c in (shift,shift+7,shift+14,shift+21))
    cross=sum(B[(r*30+c if not rot else c*30+r)] for r in (0,29)
                 for c in range(30) if c not in (shift,shift+7,shift+14,shift+21))
    service=crawled|cross
    for q in (0,29):
        assert flood(service&-service,service)==service
    row.update(rot=rot,shift=shift,tubes=len(tubes),target_braces=len(wire),braces_at_staff_epoch=0,braces_at_release=0,
               ready_at_staff_epoch=0,ready_at_release=0,unbuilt_windows=0,unstaffed_windows=0,staffed_windows=0,
               ready_animal_windows=0,gating_by_pet_or_human=0,simultaneous_seals=0,first_seal=-1,repeat_seals_after_first=0,
               bank_nonidle=0,bank_move=0,bank_build=0)
    for t in range(stop+1):
        ps,hs,ws=hist[t];pn,hn,wn=hist[t+1]
        old=sum(B[idx(w)] for w in ws);new=sum(B[idx(w)] for w in wn)
        hbits=sum({B[idx(h)] for h in hs});pbits=sum({B[idx(p)] for p in ps})
        assert not (service&old)
        reachable=flood(service&-service,ALL^old)
        assert reachable&hbits==hbits,('stranded outside bank stage',t)
        if t==stop:
            ar=reachable.bit_count()
            row.update(area_at_release=ar,residual_at_release=sum(bool(B[idx(p)]&reachable) for p in ps))
        if t<stop:
            row['bank_nonidle']+=len(wn-ws)+sum(a!=b for a,b in zip(hs,hn))
            row['bank_move']+=sum(a!=b for a,b in zip(hs,hn))
            row['bank_build']+=len(wn-ws)
        live=[0]*3
        for wi,(ls,g,k,rs,body,side) in enumerate(tubes):
            ready=side&old==side
            if ready:first.setdefault(wi,t);live[wi//12]+=1
            if old&(B[g]|B[k]):continue
            if t==stop:continue
            if body&pbits and ready:row['ready_animal_windows']+=1
            bad=hbits|pbits|wave(pbits)
            open_pet=bool(body&pbits) and not (body&hbits)
            if open_pet and (B[g]|B[k])&bad:row['gating_by_pet_or_human']+=1
            if not open_pet or (B[g]|B[k])&bad:continue
            if not ready:row['unbuilt_windows']+=1;continue
            inds={i for i,p in enumerate(ps) if B[idx(p)]&body}
            for i in inds:legal_first.setdefault(i,t)
            if (B[ls]|B[rs])&hbits!=(B[ls]|B[rs]):
                row['unstaffed_windows']+=1;continue
            row['staffed_windows']+=1
            if (B[g]|B[k])&(new^old)!=(B[g]|B[k]):continue
            inds={i for i,p in enumerate(ps) if B[idx(p)]&body}
            for i in inds:
                assert B[idx(pn[i])]&body and flood(B[idx(pn[i])],ALL^new)==body
            assert not (body&sum({B[idx(h)] for h in hn}))
            assert not (inds&captured)
            captured|=inds
            blocks[t//50]+=len(inds)
            late+=len(inds)*(t>=225)
            delays.extend(t-legal_first[i] for i in inds)
            row['simultaneous_seals']+=1
            if row['first_seal']<0:row['first_seal']=t
        if t>=35 and ready5<0 and max(live)>=5:ready5=t
    row['first_any5']=ready5
    row['captures_after_225']=late
    for i,b in enumerate(blocks):row[f'capture_block_{i}']=b
    for cut,name in ((50,'50'),(101,'staff_epoch'),(stop,'release')):
        wk={idx(p) for p in hist[cut][2]}
        row['braces_at_'+name]=len(wk&wire)
        row['ready_at_'+name]=sum(t<=cut for t in first.values())
    row['repeat_seals_after_first']=max(row['simultaneous_seals']-1,0)
    row['ready_median']=statistics.median(first.values()) if first else -1
    row['bank_capture4']=len(captured)
    row['unique_eligible_animals']=len(legal_first)
    row['unclosed_eligible_animals']=len(set(legal_first)-captured)
    row['median_latency']=statistics.median(delays) if delays else -1
    for k in range(1,6):row[f'eligible_species_{k}']=sum(kinds[i]==k for i in legal_first)
    for k in range(1,6):row[f'bank_species_{k}']=sum(kinds[i]==k for i in captured)
    return row

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','baseline','pilot','cases','output'):p.add_argument(n,type=Path)
    p.add_argument('--case-count',type=int,default=30)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    out=[]; tester=a.tester.resolve()
    for f in sorted(a.cases.glob('*.txt'))[:a.case_count]:
        case=f.read_bytes()
        state=None
        for model,mode in (('stay225',0),('rescue160',1)):
            with tempfile.TemporaryDirectory() as td:
                q=Path(td)/'meta';rescue=Path(td)/'rescue'
                command=[tester,'env',f'AHC008_BANK_LOG={q}',f'AHC008_RESCUE_LOG={rescue}',a.pilot.resolve(),'5',str(mode)]
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
                row=dict(case=f.name,model=model,score=score,time=t,
                         input_sha256=hashlib.sha256(case).hexdigest(),actions_sha256=hashlib.sha256(act).hexdigest())
                meta=q.read_text()
                n160,cutoff=map(int,rescue.read_text().split())
                head,*geom=meta.splitlines()
                rot,shift,_=map(int,head.split())
                pair=meta,act.splitlines()[:160],n160
                if state is None:state=pair;original=act
                else:assert state==pair,(f.name,'before trigger differs')
                assert cutoff==(160 if mode and n160>4 else 225)
                if mode and n160<=4:assert original==act,(f.name,'inactive branch differs')
                ps,hs,ws=hist[160]
                grid=ALL^sum(B[idx(w)] for w in ws)
                realm=flood(sum({B[idx(h)] for h in hs}),grid)
                assert sum(bool(realm&B[idx(p)]) for p in ps)==n160
                row.update(count(hist,kinds,'\n'.join([f'{rot} {shift} {cutoff}',*geom])))
                row.update(cutoff=cutoff,residual160=n160)
                ps,hs,ws=hist[-1]
                grid=ALL^sum(B[idx(w)] for w in ws)
                pbits=sum({B[idx(p)] for p in ps})
                per=[flood(B[idx(h)],grid) for h in hs]
                ca=[u.bit_count() if not u&pbits else 0 for u in per]
                row.update(cleared_humans=sum(a>0 for a in ca),n_humans=len(ca),
                           mean_clean_area_per_human=sum(ca)/len(ca),individual_clean_areas='+'.join(map(str,ca)))
                out.append(row)
        with a.output.open('w') as z:
            w=csv.DictWriter(z,dict.fromkeys(k for r in out for k in r),lineterminator='\n')
            w.writeheader();w.writerows(out)
        print(f.name,flush=True)
if __name__=='__main__':main()
