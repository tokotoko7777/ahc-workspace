"""Read-only exact wall episode and topology attribution for article v4."""
import argparse,csv,hashlib,json,collections,sys,tempfile
from pathlib import Path
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import ALL,B,idx,flood
from parallel_banks_anchored_cut_audit import buf,bits,rect

def snapshot(open_,h,p):
    remain=buf(h);hum=remain;root=0;total=0.;links=0
    while remain:
        g=flood(remain&-remain,open_);q=sum(bool(B[j]&g) for j in h)
        root|=g;remain&=~g
        total+=q*g.bit_count();links+=q*(q-1)//2
    return root,total,links

def measure(ground,h,p,addition):
    op=ground&~addition
    root,area,links=snapshot(op,h,p)
    return root,area,links

def diagnose(log,hist,cmds,name):
    meta={}
    for row in log.read_text().splitlines():
        t,m,e,ks,v,rs,gs,*coords=row.split()
        t,m,e,ks,rs,gs=map(int,(t,m,e,ks,rs,gs));coords=list(map(int,coords))
        assert len(coords)==rs+gs
        meta[t]=dict(mode=m,id=e,size=ks,value=float(v),raw=coords[:rs],goals=coords[rs:])
    hist=[[list(map(idx,pp)),list(map(idx,hh)),buf(map(idx,w))] for pp,hh,w in hist]
    episodes={}
    ticks=[]
    for t in range(300):
        pp,hh,old=hist[t];_,after,new=hist[t+1]
        meta_t=meta.get(t,dict(mode=-1,id=-1,size=-1,value=-1,raw=[],goals=[]))
        e=meta_t['id'];ms=meta_t['mode'];goal=buf(meta_t['goals'])
        if e>=0:
            assert ms==4 and goal
            epi=episodes.setdefault(e,dict(case=name,start=e,owned=[],goals=meta_t['goals'],
                                            initial_keep=meta_t['size']))
            epi['owned'].append([t,goal.bit_count(),meta_t['size']])
            assert not goal&old
        if old==new:continue
        add=new&~old
        final_by_mode={}
        raw=buf(meta_t['raw'])
        off=(add&~raw)
        if off:final_by_mode['wrapper']=off
        kept=add&raw
        if kept:
            own='late' if e>=0 else 'pocket' if ms==4 else 'lastcap' if ms in (3,5) else 'bank'
            final_by_mode[own]=kept
        assert (buf([])|sum(final_by_mode.values()))==add
        op=ALL^old
        pre,area0,links0=measure(op,after,pp,0)
        post,area1,links1=measure(op,after,pp,add)
        remain=pre&~post&~add
        after_resid=sum(bool(B[p]&post) for p in pp)
        before_resid=sum(bool(B[p]&pre) for p in pp)
        rr=[]
        while remain:
            q=flood(remain&-remain,op&~add)
            assert q&remain==q
            remain&=~q
            rr.append(dict(area=q.bit_count(),animals=[i for i,p in enumerate(pp) if B[p]&q],
                           nonrect=not rect(q)))
        lost=area0-area1
        w={}
        if len(final_by_mode)>1:
            for s,bit in final_by_mode.items():
                # Withhold only this source's walls. Marginals are not additive.
                rt,ar,li=measure(op,after,pp,add^bit)
                w[s]=dict(retained_area=ar-area1,animals_additional_reachable=sum(
                            bool(B[p]&rt) for p in pp)-after_resid)
        ticks.append(dict(case=name,turn=t,episode=e,sources={s:ds.bit_count() for s,ds in final_by_mode.items()},
                          walls=list(bits(add)),human_floor_loss=lost/len(after),
                          human_pair_splits=links0-links1,before=before_resid,after=after_resid,
                          detached=rr,marginal=w))
    for ep in episodes.values():
        last=ep['owned'][-1][0]
        goal=buf(ep['goals'])
        ep['last']=last;ep['pending_at_end']=(goal&~hist[last+1][2]).bit_count()
        ep['pending_at_final']=(goal&~hist[-1][2]).bit_count()
        source=[z for z in ticks if z['episode']==ep['start'] and 'late' in z['sources']]
        ep['actual_late_walls']=sum(x['sources']['late'] for x in source)
        ep['pure_late_floor_loss']=sum(x['human_floor_loss'] for x in source if len(x['sources'])==1)
        ep['pure_late_new_isolated']=sum(x['before']-x['after'] for x in source if len(x['sources'])==1)
    return ticks,list(episodes.values())

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','solver','inputs','reference','events','episodes','summary'):p.add_argument(n,type=Path)
    a=p.parse_args();sys.path.insert(0,str(a.harness.resolve()))
    from benchmark import replay
    from benchmark_cases import run
    with open(a.reference) as f: refs=[r for r in csv.DictReader(f) if r['model']=='article']
    ledger=[];episode=[]
    for r in refs:
        name=r['case'];case=(a.inputs/name).read_bytes()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);meta=d/'log';test=a.tester.resolve()
            out,e,rc,_=run([test,'env',f'AHC008_LATE_AREA_LOG={meta}',a.solver.resolve()],case,3.)
            assert rc==0 and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(out).hexdigest()==r['actions_sha256']
            action=d/'actions';action.write_bytes(out);t=d/'trace'
            po,pe,pr,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',action,t],case,10.)
            assert (po,pe,pr)==(out,e,0)
            lines=t.read_text().splitlines();cmds=out.decode().splitlines()
            assert replay(case.decode(),lines,cmds,detailed=True)[0]==int(r['score'])
            _,history,_=observed(case,lines);l,ep=diagnose(meta,history,cmds,name)
            assert abs(900.-sum(x['human_floor_loss'] for x in l)-float(r['mean_human_floor']))<.001
            assert sum(x['before']-x['after'] for x in l)==int(r['ever_caught'])
            assert all(x['human_floor_loss']>=0 and x['before']>=x['after'] for x in l)
            ledger+=l;episode+=ep
        print(name,flush=True)
    groups={}
    for k in ('late','lastcap','pocket','bank','wrapper','mixed'):
        rr=[x for x in ledger if (len(x['sources'])>1 if k=='mixed' else set(x['sources'])=={k})]
        broad=[x for x in rr if x['human_floor_loss']>=100]
        low_yield_broad=[x for x in broad if 2*x['after']>x['before']]
        groups[k]=dict(turns=len(rr),cases=len({x['case'] for x in rr}),
                       weighted_lost=sum(x['human_floor_loss'] for x in rr),
                       actual_captured=sum(x['before']-x['after'] for x in rr),
                       empty_detached_area=sum(z['area'] for x in rr for z in x['detached'] if not z['animals']),
                       broad_turns=len(broad),broad_cases=len({x['case'] for x in broad}),
                       low_yield_broad_turns=len(low_yield_broad),
                       low_yield_broad_cases=len({x['case'] for x in low_yield_broad}))
    s=dict(verified=len(refs),episodes=len(episode),completed=sum(not e['pending_at_end'] for e in episode),
           ever_completed=sum(not e['pending_at_final'] for e in episode),attribution=groups)
    for f,d in [(a.events,ledger),(a.episodes,episode)]:
        f.write_text(''.join(json.dumps(z,separators=(',',':'))+'\n' for z in d))
    a.summary.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))
if __name__=='__main__':main()
