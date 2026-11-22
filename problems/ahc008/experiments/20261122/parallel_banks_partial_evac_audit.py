"""Read-only exact closure opportunity at already selected late lines."""
import argparse,csv,hashlib,json,sys,tempfile,math
from pathlib import Path
from collections import Counter
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import ADJ,B,ALL,idx,flood,wave

def bits(b):
    while b:
        u=b&-b
        yield u.bit_length()-1
        b^=u

def mask(ps):
    return sum({B[q] for q in ps})

def assess(hist,actions,line,case,baseline):
    a=list(map(int,line.split()))
    t,_,start,_,n,_,mobile,inside,*_=a
    if n not in (1,2) or mobile-inside!=1 or inside<2:return None
    p0,h0,w0=hist[t]
    _,h1,w1=hist[t+1]
    pet=list(map(idx,p0));cur=list(map(idx,h0));post=list(map(idx,h1))
    p=mask(pet)
    prior=ALL^mask(map(idx,w0))
    ordinary=ALL^mask(map(idx,w1))
    cut=mask(a[16:])
    before_root=flood(B[a[15]],prior)
    assert cut.bit_count()==n and cut&prior==cut
    assert sum(bool(before_root&B[q]) for q in cur)==mobile
    entry={'case':case,'turn':t,'start':start,'jobs':n,'baseline_score':baseline,
           'planned':list(bits(cut)),'kept_cell':a[15]}
    pending=cut&ordinary
    if not pending:return {**entry,'type':'ordinary_done'}
    blockers=[]
    if pending&(p|wave(p)):blockers.append('pet')
    if pending&mask(cur):blockers.append('human_now')
    if pending&mask(post)&~mask(cur):blockers.append('ordinary_move')
    if blockers:return {**entry,'type':'illegal_or_move_conflict','blockers':blockers}
    hypothetical=ordinary&~pending
    if not hypothetical&B[a[15]]:return {**entry,'type':'main_blocked'}
    main=flood(B[a[15]],hypothetical)
    if main.bit_count()<300 or main&p:
        return {**entry,'type':'main_not_sterile_300','main_area':main.bit_count(),
                'main_pets':[i for i,q in enumerate(pet) if main&B[q]]}
    hit=[w for w,q in enumerate(post) if main&B[q]]
    if len(hit)<mobile-1:return {**entry,'type':'majority_lost'}
    available=[]
    for gate in bits(pending):
        available.append((gate,[w for w in hit if actions[t][w]=='.' and cur[w] in ADJ[gate]]))
    def assign(i,used):
        if i==len(available):return []
        g,workers=available[i]
        for w in workers:
            if w in used:continue
            z=assign(i+1,used|{w})
            if z is not None:return [(w,g)]+z
        return None
    build=assign(0,set())
    if build is None:return {**entry,'type':'no_idle_distinct_builders'}
    comp=[flood(B[q],hypothetical) for q in post]
    if len(set(comp))<2:return {**entry,'type':'no_crew_split_after_moves'}
    before_rooms=[flood(B[q],prior) for q in cur]
    reachable_before=mask(i for i,q in enumerate(pet) if any(x&B[q] for x in before_rooms))
    if not reachable_before:return {**entry,'type':'already_safe'}
    reachable_ids=[i for i,q in enumerate(pet) if any(x&B[q] for x in comp)]
    detached=[i for i in bits(reachable_before) if i not in reachable_ids]
    if not detached:return {**entry,'type':'no_actual_pet_separation'}
    rooms={}
    for i in detached:
        pen=flood(B[pet[i]],hypothetical)
        rooms.setdefault(pen,[]).append(i)
    scores=[(r.bit_count()/900.)*2.**(-sum(bool(r&B[q]) for q in pet)) for r in comp]
    q=1e8*sum(scores)/len(comp)
    score=math.floor(q)+int(q-math.floor(q)>=.5)
    return {**entry,'type':'B' if reachable_ids else 'A','score_at_closure':score,'delta_to_normal_final':score-baseline,
            'pending':list(bits(pending)),'build':build,'ordinary':actions[t], 'main_area':main.bit_count(),
            'main_humans':hit,'human_areas':[r.bit_count() for r in comp],
            'reachable_after':reachable_ids,'detached':detached,
            'pet_components':[{'area':r.bit_count(),'pets':v} for r,v in rooms.items()]}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','observer','inputs','ref','out','summary'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with a.ref.open() as f:ref=[r for r in csv.DictReader(f) if r['model']=='narrow']
    out=[]
    for r in ref:
        name=r['case'];case=(a.inputs/name).read_bytes()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);dump=d/'dump';trace=d/'trace';af=d/'actions'
            test=a.tester.resolve()
            acts,e,rc,_=run([test,'env',f'AHC008_EVAC_AUDIT={dump}',a.observer.resolve()],case,3.)
            assert rc==0 and e.strip()==('Score = '+r['score']).encode()
            assert r['actions_sha256']==hashlib.sha256(acts).hexdigest()
            af.write_bytes(acts)
            pb,e2,rc2,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py','--playback',af,trace],case,10.)
            assert (pb,e2,rc2)==(acts,e,0)
            lines=trace.read_text().splitlines();cmd=acts.decode().splitlines()
            assert replay(case.decode(),lines,cmd,detailed=True)[0]==int(r['score'])
            _,hist,_=observed(case,lines)
            for line in dump.read_text().splitlines() if dump.exists() else []:
                row=assess(hist,cmd,line,name,int(r['score']))
                if row:out.append(row)
    groups={}
    for tag in ('A','B'):
        q=[r for r in out if r['type']==tag]
        best={}
        for r in q:best[r['case']]=max(best.get(r['case'],-10**9),r['delta_to_normal_final'])
        groups[tag]=dict(ticks=len(q),episodes=len({(r['case'],r['start']) for r in q}),
             cases=len(best),positive_constructive_cases=sum(d>0 for d in best.values()),
             best_delta_by_case=best,optimistic_positive_only=sum(max(0,d) for d in best.values()))
    small=[r for r in out if r['type']=='main_not_sterile_300']
    summary=dict(verified=len(ref),screened_ticks=len(out),screened_cases=len({r['case'] for r in out}),
                 filters=dict(Counter(r['type'] for r in out)),groups=groups,
                 blocking_ingredients=dict(Counter(b for r in out for b in r.get('blockers',[]))),
                 main_below_300=sum(r['main_area']<300 for r in small),
                 main_contains_pets=sum(bool(r['main_pets']) for r in small),
                 sterile_but_small_max=max((r['main_area'] for r in small if not r['main_pets']),default=0))
    a.out.write_text(''.join(json.dumps(r,separators=(',',':'))+'\n' for r in out))
    a.summary.write_text(json.dumps(summary,indent=2)+'\n')
    print(json.dumps(summary,indent=2))

def fixture():
    opened={(r,c) for r in range(11) for c in range(30)}|{(11,15)}|{(12,c) for c in range(12,19)}
    walls=frozenset((r,c) for r in range(30) for c in range(30) if (r,c) not in opened)
    hs=((9,5),(11,15),(12,13))
    meta=[0,0,0,0,1,0,3,2,0,0,1,0,0,1,1,345,375]
    for ps,label in [(((12,18),),'A'),(((12,18),(12,12)),'B')]:
        r=assess([(ps,hs,walls)]*2,['...'],' '.join(map(str,meta)),'fixture',0)
        assert r['type']==label and r['main_area']==331 and r['build']==[(1,375)],r

if __name__=='__main__':
    if sys.argv[1:]==['--fixture']:fixture()
    else:main()
