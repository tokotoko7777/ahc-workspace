"""Audit v7 post-handoff genuine secondary crew on unchanged official traces."""
import argparse,csv,hashlib,itertools,json,sys,tempfile,collections
from pathlib import Path
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import ALL,B,ADJ,idx,flood,wave
from parallel_banks_anchored_cut_audit import buf

def candidate(t,s,builds,x,actors,route):
    eta=len(route[0])-1;u=t+eta
    a,b=actors;w=s[t][2]
    touched=0;paths=dict(zip(actors,route,strict=True))
    for z in range(t,u+1):
        p,h,_=s[z]
        p=buf(p);hh=list(h);after=list(s[z+1][1])
        ao=z-t
        for i,pp in paths.items():
            hh[i]=pp[ao];after[i]=pp[ao+1] if z<u else pp[ao]
        if buf(hh)&w:return None
        forbidden=p|wave(p)|buf(hh)
        jobs={i:q for i,q in builds[z].items() if i not in paths}
        for i,q in builds[z].items():
            if i in paths and (z<u or q!=(x.g if i==a else x.z)):touched+=1
        mod=buf(jobs.values())
        if (w|forbidden)&mod:return None
        if z==u:
            if (x.gates|x.body)&w or x.wall&w!=x.wall:return None
            if x.gates&(w|forbidden|mod):return None
            mod|=x.gates
        w|=mod
        if buf(after)&w:return None
        root=flood(B[after[0]],ALL^w)
        if root&buf(after)!=buf(after):return None
    op=ALL^w
    old=s[u][0]
    seen=[i for i,q in enumerate(old) if B[q]&x.body]
    if not seen:return None
    if flood(B[old[seen[0]]],op)!=x.body:return None
    assert not x.body&root
    return dict(actors=actors,route=route,replaced_secondary_walls=touched,pets=seen)

def paths_to(start,need,n,allows):
    if not n:return [[start]] if start==need else []
    prev=[[start]]
    for step in range(n):
        next_=[]
        for ps in prev:
            for q in (ps[-1],*ADJ[ps[-1]]):
                if B[q]&allows and (abs(q//30-need//30)+abs(q%30-need%30)<=n-step-1):
                    next_.append(ps+[q])
        prev=next_
    return prev

def examine(name,log,tubes,hist,cmds):
    s=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in hist]
    roster={}
    for line in log.read_text().splitlines():
        t,m=map(int,line.split());assert 160<=t<205
        roster[t]=m
    builds=[]
    for t,row in enumerate(cmds):
        d={i:idx(step(divmod(s[t][1][i],30),STEPS[c.upper()])) for i,c in enumerate(row) if c.islower()}
        builds.append(d)
    final=flood(buf(s[-1][1]),ALL^s[-1][2])
    missed={i for i,q in enumerate(s[-1][0]) if B[q]&final}
    rows=[];counts=collections.Counter()
    for t,r in sorted(roster.items()):
        _,h,w=s[t]
        counts['real_dual_turns']+=1
        ready=[x for x in tubes if x.wall&w==x.wall and not (x.gates|x.body)&w]
        body=buf(q for x in tubes if x.wall&w==x.wall for q in x.inside)
        ext=(ALL^w)&~body
        for eta in (0,1,2):
            u=t+eta
            if any(z not in roster for z in range(t,u+1)):continue
            common=r
            for z in range(t+1,u+1):common&=roster[z]
            if common.bit_count()<2:continue
            for x in ready:
                p=s[u][0];pp=buf(p)
                if not pp&x.body or x.gates&(pp|wave(pp)):continue
                counts[f'factual_pet_window_{eta}']+=1
                options=[]
                for need in (x.l,x.r):
                    a=[]
                    for i,p0 in enumerate(h):
                        if not (common>>i&1) or abs(p0//30-need//30)+abs(p0%30-need%30)>eta:continue
                        start_inside=next((y.body for y in tubes if y.body&B[p0] and not y.wall&~w),0)
                        dd=paths_to(p0,need,eta,ext|((ALL^w)&start_inside))
                        a.extend((i,q) for q in dd)
                    options.append(a)
                known=None
                for (a,ar),(b,br) in itertools.product(*options):
                    if a==b:continue
                    if eta>0 and ar==[ar[0]]*len(ar) and br==[br[0]]*len(br):continue
                    q=candidate(t,s,builds,x,[a,b],[ar,br])
                    if q is None:continue
                    if known is None or q['replaced_secondary_walls']<known['replaced_secondary_walls']:known=q
                if known:
                    known.update(case=name,turn=t,eta=eta,tube=x.k,
                                 otherwise_final_missed=[i for i in known['pets'] if i in missed])
                    rows.append(known)
    return rows,counts

def selfcheck(Tube):
    # True straight middle in one row. Two actual p2 builders, intact braces,
    # one real safe cat cell; separated after this exact legal pair.
    row=15;a=5
    body=tuple(row*30+c for c in range(a+2,a+6))
    walls=buf(s*30+c for s in (row-1,row+1) for c in range(a+2,a+6))
    x=Tube(0,0,row,row*30+a,row*30+a+1,row*30+a+6,row*30+a+7,body,walls)
    h=[x.l,x.r,710,820,851,884]
    p=[body[1]]
    s=[(p,h,walls)]*2
    assert candidate(0,s,[{}],x,[0,1],[[x.l],[x.r]])['pets']==[0]
    p=[body[0]];s=[(p,h,walls)]*2
    assert candidate(0,s,[{}],x,[0,1],[[x.l],[x.r]]) is None

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','bridge_dir','tester','solver','cases','reference','out','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path[:0]=[str(a.harness.resolve()),str(a.bridge_dir.resolve())]
    from benchmark_cases import run
    from benchmark import replay
    from bridge import parse_geometry,Tube
    selfcheck(Tube)
    with open(a.reference) as f:refs=[r for r in csv.DictReader(f) if r['model']=='article']
    rr=[];ss=collections.Counter();cases=set()
    for r in refs:
        name=r['case'];data=(a.cases/name).read_bytes();test=a.tester.resolve()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);log=d/'log';ge=d/'geo';tr=d/'tape';act=d/'act'
            o,e,rc,_=run([test,'env',f'AHC008_SECONDARY_BANK_LOG={log}',f'AHC008_BANK_LOG={ge}',a.solver.resolve()],data,3.)
            assert not rc and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tr],data,10.)
            assert (po,pe,pr)==(o,e,0)
            tape=tr.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(data.decode(),tape,cmds,detailed=True)[0]==int(r['score'])
            _,history,_=observed(data,tape)
            _,_,tubes=parse_geometry(ge)
            q,v=examine(name,log,tubes,history,cmds);rr+=q;ss.update(v)
            if v['real_dual_turns']:cases.add(name)
        print(name,flush=True)
    dist=lambda qs,k:len({(x['case'],p) for x in qs for p in x[k]})
    report={}
    for h in range(3):
        v=[q for q in rr if q['eta']==h]
        miss=[q for q in v if q['otherwise_final_missed']]
        no=[q for q in miss if not q['replaced_secondary_walls']]
        report[h]=dict(witnesses=len(v),cases=len({q['case'] for q in v}),animals=dist(v,'pets'),
          final_missed=dist(v,'otherwise_final_missed'),final_missed_cases=len({q['case'] for q in miss}),
          final_missed_no_secondary_wall_lost=dist(no,'otherwise_final_missed'),
          lost_wall_hist=dict(collections.Counter(q['replaced_secondary_walls'] for q in v)))
    out=dict(verified=len(refs),actual_dual_cases=len(cases),counts=ss,by_eta=report,
             final_missed_total=dist(rr,'otherwise_final_missed'))
    a.out.write_text(''.join(json.dumps(x,separators=(',',':'))+'\n' for x in rr))
    a.summary.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out,indent=2))
if __name__=='__main__':main()
