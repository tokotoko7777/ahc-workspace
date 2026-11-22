"""Exact official/replayed AHC008 paired benchmark centered on full capture."""
import argparse,csv,hashlib,json,re,statistics,sys,tempfile
from pathlib import Path

def measure_history(hist,kinds,B,ALL,idx,flood,release):
    all_clear=-1; events=[]; caught=set(); sums={}; footprints={}
    residual_groups=[]
    safe_area=people_area=human_count=0
    final_safe_union=0; capacities={}
    moments={160,225,300,release}
    for t,(pets,people,walls) in enumerate(hist):
        op=ALL^sum(B[idx(w)] for w in walls)
        hbits=sum({B[idx(h)] for h in people})
        root=flood(hbits,op)
        assert root&hbits==hbits
        residual={i for i,p in enumerate(pets) if root&B[idx(p)]}
        residual_groups.append(len(residual))
        new=set(range(len(pets)))-caught-residual
        assert not caught & residual
        for i in sorted(new):
            part=flood(B[idx(pets[i])],op)
            footprints[i]=part
            coords=[divmod(j,30) for j in range(900) if part&B[j]]
            ar=len(coords)
            rect=ar==(max(a for a,b in coords)-min(a for a,b in coords)+1)*(max(b for a,b in coords)-min(b for a,b in coords)+1)
            events.append((t,i,kinds[i],ar,int(not rect)))
            caught.add(i)
        if not residual and all_clear<0:all_clear=t
        if t in moments:
            group_areas=[flood(B[idx(h)],op).bit_count() for h in people]
            capacity_float=1e8 * sum(ar/900. for ar in group_areas) / len(people)
            cap=int(capacity_float + .5)
            if t in (160,225,300): capacities[f'area_capacity_turn{t}']=cap
            if t==release: capacities['area_capacity_release']=cap
        if t==300:
            petbits=sum({B[idx(p)] for p in pets})
            for p in people:
                c=flood(B[idx(p)],op)
                ar=c.bit_count()
                people_area+=ar
                if not c&petbits:
                    safe_area+=ar
                    human_count+=1
                    final_safe_union|=c
            for i,p in enumerate(pets):
                if i in residual:continue
                final=flood(B[idx(p)],op)
                assert final==footprints[i],(i,final.bit_count(),footprints[i].bit_count())
                ar=final.bit_count()
                sums[ar]=sums.get(ar,0)+1
    return dict(animals=len(kinds),humans=len(hist[-1][1]),first_all_clear=all_clear,
                reported_release=release, **capacities,
                all_clear=int(all_clear>=0),ever_caught=len(caught),
                final_reachable=residual_groups[-1],final_exact4=sums.get(4,0),
                exact4_first=sum(ar==4 for t,i,k,ar,rr in events),
                exact_nonrect4to48_first=sum(rr and 4<=ar<=48 for t,i,k,ar,rr in events),
                residual_turn100=residual_groups[100],residual_turn160=residual_groups[160],
                residual_turn225=residual_groups[225],residual_turn275=residual_groups[275],
                clean_people=human_count,clean_union=final_safe_union.bit_count(),
                mean_human_floor=round(people_area/len(hist[-1][1]),3),
                mean_clean_floor=round(safe_area/len(hist[-1][1]),3),
                captures=';'.join(':'.join(map(str,e)) for e in events))

def play(tester,exe,extra,input_data,run,replay,observed,check,harness):
    with tempfile.TemporaryDirectory() as tmpdir:
        tmp=Path(tmpdir)
        a=tmp/'actions';z=tmp/'trace';p=tmp/'phase'; q=tmp/'release'
        bridge=tmp/'bridge'; coverage=tmp/'coverage'
        cmd=[tester,'env','AHC008_ARTICLE_LOG='+str(p),'AHC008_RESCUE_LOG='+str(q),'AHC008_BRIDGE_LOG='+str(bridge),'AHC008_CORDON_LOG='+str(coverage),exe,*extra]
        action,err,rc,wall=run(cmd,input_data,4.5)
        sm=re.fullmatch(rb'Score = (\d+)\s*',err)
        assert rc==0 and sm,(exe,rc,err)
        a.write_bytes(action)
        b,newerr,newrc,_=run([tester,sys.executable,harness/'benchmark_cases.py','--playback',a,z],input_data,12.)
        assert (b,newerr,newrc)==(action,err,0)
        moves=z.read_text().splitlines()
        actions=action.decode().splitlines()
        assert len(actions)==300
        score,*_=replay(input_data.decode(),moves,actions,detailed=True)
        assert score==int(sm[1])
        kinds,hist,_=observed(input_data,moves)
        assert len(hist)==301
        events=[];actives={}
        if bridge.exists():
            for r in bridge.read_text().splitlines():
                t,k,actor,tube,f,g,ss=map(int,r.split())
                origin=hist[t][1][actor];end=hist[t+1][1][actor]
                assert 30*origin[0]+origin[1]==f
                if k==0:
                    assert 101<=t<=152 and ss>0 and not actives
                    actives[tube]=(t,actor,g)
                if k in (1,2):
                    begin,i,dest=actives.pop(tube)
                    assert i==actor and t<=159
                    if k==1:assert t==begin+7 and f==dest
                if k==3:assert 30*end[0]+end[1]==g and actions[t][actor].isupper()
                if k==4:assert actions[t][actor]=='.'
                events.append(r)
        assert not actives or check(hist,kinds)['all_clear']
        return dict(actions_sha256=hashlib.sha256(action).hexdigest(),score=score,wall_sec=wall,
                    cordon=coverage.read_text().strip() if coverage.exists() else '',
                    telemetry=p.read_text().strip().replace('\n',';') if p.exists() else '',
                    bridge=';'.join(events),prefix100=hashlib.sha256(('\n'.join(actions[:101])+'\n').encode()).hexdigest(),
                    **check(hist,kinds,int(q.read_text().split()[1]) if q.exists() else -1))

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in 'harness tester baseline candidate cases output'.split():
        p.add_argument(n,type=Path)
    p.add_argument('--n',type=int,default=30)
    p.add_argument('--base-arg',action='append',default=[])
    p.add_argument('--candidate-arg',action='append',default=[])
    x=p.parse_args()
    sys.path.insert(0,str(x.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    from guarded_entry_trace import observed
    from stochastic_lock_audit import ALL,B,idx,flood
    cases=sorted(x.cases.glob('*.txt'))[:x.n]
    assert len(cases)==x.n
    f=lambda h,k,r:measure_history(h,k,B,ALL,idx,flood,r)
    r=[]
    for n in cases:
        v=n.read_bytes()
        hh=hashlib.sha256(v).hexdigest()
        for key,exe,opts in [('baseline',x.baseline,x.base_arg),('article',x.candidate,x.candidate_arg)]:
            q=play(x.tester.resolve(),exe.resolve(),opts,v,run,replay,observed,f,x.harness.resolve())
            r.append(dict(case=n.name,model=key,input_sha256=hh,**q))
        with x.output.open('w') as w:
            z=csv.DictWriter(w,list(r[0]),lineterminator='\n');z.writeheader();z.writerows(r)
        print(n.name,flush=True)
    arms={k:[s for s in r if s['model']==k] for k in ('baseline','article')}
    metrics=('score','all_clear','final_reachable','final_exact4','exact4_first','exact_nonrect4to48_first',
             'area_capacity_turn160','area_capacity_turn225','area_capacity_turn300','area_capacity_release',
             'residual_turn100','residual_turn160','residual_turn225','residual_turn275','clean_people','clean_union')
    res={m:{
        'sum':{k:sum(a[k] for a in rows) for k in metrics},
        'median_clear_turn':statistics.median(v) if (v:=[a['first_all_clear'] for a in rows if a['all_clear']]) else None,
        'median_wall':statistics.median(a['wall_sec'] for a in rows),
        'max_wall':max(a['wall_sec'] for a in rows)} for m,rows in arms.items()}
    print(json.dumps(res,indent=2))

if __name__=='__main__':main()
