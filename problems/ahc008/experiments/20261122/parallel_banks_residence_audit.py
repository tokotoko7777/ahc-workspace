"""Read-only evaluation of actual-ETA passive forecast vs Manhattan mass."""
import argparse,csv,gzip,hashlib,itertools,json,sys,tempfile
from collections import Counter,defaultdict
from pathlib import Path
import numpy as np
from guarded_entry_trace import observed
from stochastic_lock_audit import ADJ,ALL,B,idx,flood,wave

V=900
NB=tuple(tuple(v for v in ADJ[i]) for i in range(V))
def index_values(mask):
    while mask:
        k=mask&-mask
        yield k.bit_length()-1
        mask^=k

def dists(seed,open_):
    out=[-1]*V
    layer=seed&open_;seen=layer;i=0
    while layer:
        for p in index_values(layer):out[p]=i
        layer=wave(layer)&open_&~seen
        seen|=layer;i+=1
    return out

def transitions(open_):
    ori=[];dest=[];pr=[]
    for u in index_values(open_):
        avail=[v for v in NB[u] if open_&B[v]]
        if not avail:avail=[u]
        ori.extend([u]*len(avail));dest.extend(avail);pr.extend([1./len(avail)]*len(avail))
    return np.array(ori,dtype=np.intp),np.array(dest,dtype=np.intp),np.array(pr)

def turn_probs(kernel,ix,steps):
    ori,end,p=kernel
    mass=np.zeros(V);mass[ix]=1
    for _ in range(steps):
        mass=np.bincount(end,weights=mass[ori]*p,minlength=V)
    assert abs(mass.sum()-1)<1e-10
    return mass

def parse(row):
    t,g,c,f=map(int,row[:4]);v=float(row[4]);x,r,w,n=map(int,row[5:9])
    walls=list(map(int,row[9:9+n]))
    inner=list(map(int,row[10+n:]))
    assert len(inner)==int(row[9+n])
    return t,g,c,f,v,x,r,w,walls,inner

def judge_case(name,sp,hist,rows):
    grid=[ALL^sum(B[idx(w)] for w in hist[t][2]) for t in range(301)]
    c_by_turn={}
    cache={}
    common={ }
    for t,g,chosen,flow,oldscore,center,radius,crew,walls,inside in map(parse,rows):
        open_=grid[t]
        cut=sum(B[p] for p in walls if open_&B[p])
        own=sum(B[p] for p in inside if open_&B[p])
        after=open_&~cut
        target=flood(own&after,after)
        if not cut or not target or target.bit_count()>96:continue
        positions=[idx(h) for j,h in enumerate(hist[t][1]) if crew>>j&1]
        if not positions:continue
        start=sum({B[q] for q in positions})
        if (t,crew) not in c_by_turn:c_by_turn[t,crew]=dists(start,open_)
        dist=c_by_turn[t,crew]
        nearest=[min((dist[q] for q in NB[v] if dist[q]>=0 and not target&B[q]),default=999)
                 for v in index_values(cut)]
        eta=max(max(nearest)+1,(cut.bit_count()+len(positions)-1)//len(positions))
        stop=t+eta
        if stop>=300:continue
        if chosen:common[g]=eta
        common_eta=common.get(g,-1)
        joint_pets=hist[t+common_eta][0] if common_eta>=0 else None
        start_pets=hist[t][0]; later_pets,later_humans,_=hist[stop]
        cut_live=cut&grid[stop];safe_orig=target&~(cut|wave(cut))
        safe_actual=target&~(cut_live|wave(cut_live))
        all_later=sum({B[idx(p)] for p in later_pets})
        mx_people=sum({B[idx(p)] for p in later_humans})
        global_gate_ok=not ((all_later|wave(all_later)|mx_people)&cut_live)
        cohort=flood(own,open_)&flood(start,open_)
        key=t,open_
        if key not in cache:cache[key]=transitions(open_),{}
        kernel,memo=cache[key]
        pets=[]
        for i,p in enumerate(start_pets):
            ix=idx(p);kind=sp[i]
            if not cohort&B[ix]:continue
            jx=idx(later_pets[i]); d=abs(ix//30-center//30)+abs(ix%30-center%30)
            mass=(1.24 if kind==1 else .65 if kind==4 else 1.)*max(0.,1.5-d/(radius+1.)) if not flow or d<=radius+2 else 0.
            datum=dict(pet=i,species=kind,legacy=mass,now=int(bool(target&B[ix])),
                       held_inside=int(bool(target&B[jx])),held_orig_safe=int(bool(safe_orig&B[jx])),
                       held_actual_safe=int(bool(safe_actual&B[jx])),at_eta_position=jx)
            if joint_pets is not None:
                j_common=idx(joint_pets[i])
                datum.update(joint_inside=int(bool(target&B[j_common])),
                             joint_orig_safe=int(bool(safe_orig&B[j_common])))
            if kind<=3:
                if common_eta>=0:
                    key_common=(ix,kind*common_eta)
                    if key_common not in memo:memo[key_common]=turn_probs(kernel,ix,kind*common_eta)
                    pp=memo[key_common]
                    datum.update(pred_joint=float(pp[list(index_values(target))].sum()),
                                 pred_joint_safe=float(pp[list(index_values(safe_orig))].sum()))
                k=(ix,kind*eta)
                if k not in memo:memo[k]=turn_probs(kernel,ix,kind*eta)
                pd=memo[k]
                datum.update(pred_inside=float(pd[list(index_values(target))].sum()),
                             pred_orig_safe=float(pd[list(index_values(safe_orig))].sum()))
            pets.append(datum)
        if not pets:continue
        real=next((u for u in range(t+1,min(300,t+26)) if not (cut&grid[u])),None) if chosen else None
        passives=[z for z in pets if z['species']<=3]
        directed=[z for z in pets if z['species']>=4]
        yield dict(case=name,turn=t,selector=g,chosen=chosen,flow=flow,oldscore=oldscore,center=center,radius=radius,crew=crew,
                   jobs=cut.bit_count(),area=target.bit_count(),eta=eta,common_eta=common_eta,real_observed_closure_turn=real if real is not None else -1,
                   global_gate_ok=int(global_gate_ok),passive=len(passives),directed=len(directed),
                   directed_inside_held=sum(z['held_inside'] for z in directed),pets=pets,
                   fields={k:sum(z[k] for z in passives) for k in ('now','legacy','pred_inside','pred_orig_safe',
                       'held_inside','held_orig_safe','held_actual_safe')},
                   joint_fields={k:sum(z[k] for z in passives) for k in ('joint_inside','joint_orig_safe','pred_joint','pred_joint_safe')}
                   if common_eta>=0 else {},
                   wall_cells=list(index_values(cut)),pen_cells=list(index_values(target)))


def compare(groups,label,legacy,forecast):
    pair=Counter();case_map=defaultdict(Counter)
    for case,q in groups:
        for a,b in itertools.combinations(q,2):
            if a[label]==b[label]:continue
            if a[label]<b[label]:a,b=b,a
            base=1 if a[legacy]>b[legacy] else 0 if a[legacy]<b[legacy] else .5
            model=1 if a[forecast]>b[forecast] else 0 if a[forecast]<b[forecast] else .5
            for rec in (pair,case_map[case]):
                rec['pairs']+=1;rec['legacy_correct']+=base;rec['forecast_correct']+=model
    if not pair['pairs']:return {}
    return dict(pairs=pair['pairs'],cases=len(case_map),
        legacy_concordance=pair['legacy_correct']/pair['pairs'],forecast_concordance=pair['forecast_correct']/pair['pairs'],
        cases_better=sum(v['forecast_correct']>v['legacy_correct'] for v in case_map.values()),
        cases_worse=sum(v['forecast_correct']<v['legacy_correct'] for v in case_map.values()),
        cases_tied=sum(v['forecast_correct']==v['legacy_correct'] for v in case_map.values()),
        case_results=dict(case_map))

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','instrumented','cases','control','output','json'):
        p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with a.control.open() as f:
        ref=[r for r in csv.DictReader(f) if r['model']=='narrow']
    out=[];total=0;totalChosen=0
    for r in ref:
        path=a.cases/r['case'];case=path.read_bytes()
        with tempfile.TemporaryDirectory() as d:
            b=Path(d)/'pocket'
            cmd=[a.tester.resolve(),'env',f'AHC008_RESIDENCE_AUDIT={b}',a.instrumented.resolve()]
            acts,err,rc,_=run(cmd,case,3.)
            assert rc==0 and err.strip()==('Score = '+r['score']).encode(),(r['case'],err)
            assert hashlib.sha256(acts).hexdigest()==r['actions_sha256'],r['case']
            af=Path(d)/'actions';trace=Path(d)/'trace';af.write_bytes(acts)
            play,err2,rc2,_=run([a.tester.resolve(),sys.executable,a.harness.resolve()/'benchmark_cases.py',
                                '--playback',af,trace],case,10.)
            assert (play,err2,rc2)==(acts,err,0)
            lines=trace.read_text().splitlines()
            assert int(r['score'])==replay(case.decode(),lines,acts.decode().splitlines(),detailed=True)[0]
            sp,hist,_=observed(case,lines)
            rows=[q.split() for q in b.read_text().splitlines()] if b.exists() else []
            total+=len(rows); totalChosen+=sum(q[2]=='1' for q in rows)
            assert all(int(q[0])>=int(r['release']) for q in rows)
            out.extend(judge_case(r['case'],sp,hist,rows))
        print(r['case'],flush=True)
    with a.output.open('w') as f:
        for row in out: f.write(json.dumps(row,separators=(',',':'))+'\n')
    by=defaultdict(list); keep=[]; chosen=[]
    for r in out:
        q={**r['fields'],**r['joint_fields'],'oldscore':r['oldscore'],'jobs':r['jobs'],'eta':r['eta'],'area':r['area'],'chosen':r['chosen']}
        if r['passive']:
            by[r['case'],r['selector']].append(q)
            if r['chosen']:
                chosen.append(r)
                keep.append((r['case'],[u for u in r['pets'] if u['species']<=3]))
    gr=[(case,items) for (case,g),items in by.items()]
    summary=dict(verified_cases=len(ref),all_enumerated=total,chosen_enumerated=totalChosen,eligible_candidates=len(out),
        chosen_passive=len(chosen),chosen_observed_complete_25=sum(r['real_observed_closure_turn']>=0 for r in chosen),
        chosen_turn_ready_legal=sum(r['global_gate_ok'] for r in chosen),
        chosen_median_eta=float(np.median([r['eta'] for r in chosen])),
        alternative_inside=compare(gr,'held_inside','legacy','pred_inside'),
        alternative_safe_orig=compare(gr,'held_orig_safe','legacy','pred_orig_safe'),
        alternative_safe_actual=compare(gr,'held_actual_safe','legacy','pred_orig_safe'),
        alternative_original_rank=compare(gr,'held_inside','oldscore','pred_inside'),
        chosen_pets_inside=compare(keep,'held_inside','legacy','pred_inside'),
        chosen_pets_safe_orig=compare(keep,'held_orig_safe','legacy','pred_orig_safe'),
        chosen_pets_safe_actual=compare(keep,'held_actual_safe','legacy','pred_orig_safe'))
    checks={}
    for name, condition in [
       ('all',lambda a,b:True),('same_jobs',lambda a,b:a['jobs']==b['jobs']),
       ('eta_close',lambda a,b:abs(a['eta']-b['eta'])<=2),
       ('jobs_eta',lambda a,b:a['jobs']==b['jobs'] and abs(a['eta']-b['eta'])<=2),
       ('matched',lambda a,b:a['jobs']==b['jobs'] and abs(a['eta']-b['eta'])<=2 and a['now']==b['now']
          and max(a['area'],b['area'])<=1.2*min(a['area'],b['area'])),
       ('neararea_same_now',lambda a,b:a['now']==b['now'] and max(a['area'],b['area'])<=1.2*min(a['area'],b['area'])),
       ('chosen_vs_alternative',lambda a,b:a['chosen']!=b['chosen'])]:
        pairs=[(c,[a,b]) for c,opts in gr for a,b in itertools.combinations(opts,2)
               if 'joint_inside' in a and 'joint_inside' in b and condition(a,b)]
        rec={}
        for label,model in [('held_inside','pred_inside'),('held_orig_safe','pred_orig_safe'),
                            ('joint_inside','pred_joint'),('joint_orig_safe','pred_joint_safe')]:
            for base in ('legacy','area','now'):
                rec[label+'_'+base]=compare(pairs,label,base,model)
        checks[name]=rec
    summary['controls']=checks
    a.json.write_text(json.dumps(summary,indent=2)+'\n')
    print(json.dumps({k:({kk:vv for kk,vv in x.items() if kk!='case_results'} if isinstance(x,dict) else x) for k,x in summary.items() if k!='controls'},indent=2))

if __name__=='__main__':main()
