"""Can a tactical target's planned wall prefix pay for an exact-four shelter side?"""
import argparse,csv,itertools,re,sys,tempfile
from pathlib import Path
from collections import Counter
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,ADJ,SPEED,idx,flood,wave,cells
from quad_side_reuse_audit import BASE,build_catalog
H=6
COLS=('two_shared_planned','entire_side_planned','side_materializes_by6',
      'side_and_2to3_extra_cells','bounded_visible_pet','observed_pet_in_body_when_side_ready',
      'all_extra_stances_one_step','all_extra_stances_no_relocation','no_relocation_and_legal','baseline_wall_jobs_to_override')
def roster(need,hs,walk,block=0):
    ng=list(cells(need))
    if not ng:return []
    if need&block or len(ng)>len(hs):return None
    stance=[{i for i,a in enumerate(hs) if any(a&B[q] and walk&B[q] for q in ADJ[g])} for g in ng]
    def pick(k,used):
        if k==len(stance):return []
        for i in sorted(stance[k]-used):
            next=pick(k+1,used|{i})
            if next is not None:return [i]+next
        return None
    return pick(0,set())

def test(geo,hist,kinds,md,trace):
    keys=Counter()
    for t,plans in md.items():
        if not plans:continue
        pp,hh,wall=hist[t]
        ps=list(map(idx,pp))
        old=sum(B[idx(x)] for x in wall)
        hi=sum({B[idx(x)] for x in hh})
        reach=flood(hi,ALL^old)
        earliest=[sum(B[idx(x)] for x in hist[t+h][2]) for h in range(H+1)]
        reach_pet=0
        for p,k in zip(ps,kinds):
            r=B[p]
            for _ in range(H*SPEED[k]):r|=wave(r)&~old
            reach_pet|=r
        for body,(shell,sides) in geo.items():
            if (body|shell)&hi or body&old or not body&reach:continue
            if max((shell&p).bit_count() for p in plans)>=2:keys['two_shared_planned']+=1
            found={side for side in sides if any(side&p==side for p in plans)}
            if not found:continue
            keys['entire_side_planned']+=1
            future=[h for h,new in enumerate(earliest) if any(side&new==side for side in found)]
            if not future:continue
            keys['side_materializes_by6']+=1
            for level,h in enumerate(future):
                need=shell&~earliest[h]
                if not 2<=need.bit_count()<=3:continue
                keys['side_and_2to3_extra_cells']+=1
                if body&reach_pet:keys['bounded_visible_pet']+=1
                pp2,hh2,ws2=hist[t+h]
                pats=sum({B[idx(x)] for x in pp2}); hbs=sum({B[idx(x)] for x in hh2})
                if not (body&pats) or body&(hbs|earliest[h]):break
                keys['observed_pet_in_body_when_side_ready']+=1
                walk=ALL&~(body|shell|earliest[h])
                ho=[B[idx(x)] for x in hh2]
                if roster(need,[a|wave(a)&walk for a in ho],walk) is not None:keys['all_extra_stances_one_step']+=1
                if roster(need,ho,walk) is not None:keys['all_extra_stances_no_relocation']+=1
                crew=roster(need,ho,walk,pats|wave(pats)|hbs)
                if crew is not None:
                    keys['no_relocation_and_legal']+=1
                    keys['baseline_wall_jobs_to_override']+=sum(trace[2*(t+h)][i].islower() for i in crew)
                break
    return keys

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','base','instrumented','cases'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester=a.tester.resolve()
    playback=a.harness.resolve()/'benchmark_cases.py'
    geo={f:build_catalog(f) for f in BASE}
    out=csv.writer(sys.stdout,lineterminator='\n');out.writerow(['case','shape','logged_plan_turns',*COLS])
    for case in sorted(a.cases.glob('*.txt')):
        payload=case.read_bytes()
        expected=run([tester,a.base.resolve()],payload,3.)[:3]
        with tempfile.TemporaryDirectory() as d:
            intent=Path(d)/'intent'
            cmd,err,rc,_=run([tester,'env',f'AHC008_TARGET_LIST={intent}',a.instrumented.resolve()],payload,3.)
            match=re.fullmatch(rb'Score = (\d+)\s*',err)
            assert rc==0 and match and (cmd,err,rc)==expected,(case,rc,err)
            md={}
            for line in intent.read_text().splitlines():
                t,*a1=line.split('|')
                md[int(t)]=[sum({B[int(w)] for w in s.split()[1:]}) for s in a1]
            assert len(md)==68
            path=Path(d)/'actions';path.write_bytes(cmd)
            trace=Path(d)/'trace'
            response,e,c,_=run([tester,sys.executable,playback,'--playback',path,trace],payload,10.)
            assert c==0 and response==cmd and e==err
            data=trace.read_text().splitlines()
        score,*_=replay(payload.decode(),data,cmd.decode().splitlines(),detailed=True)
        assert score==int(match[1])
        kinds,hist,_=observed(payload,data)
        for i,g in geo.items():
            c=test(g,hist,kinds,md,data)
            out.writerow([case.name,i,sum(bool(p) for p in md.values()),*[c[k] for k in COLS]])
        sys.stdout.flush()
if __name__=='__main__':main()
