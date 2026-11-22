"""Observe one-turn already-aligned normal crew eligibility; evaluate no live policy."""
import argparse,csv,hashlib,json,subprocess,sys,tempfile
from collections import defaultdict
from pathlib import Path
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import B,ALL,idx,wave,flood
from parallel_banks_dog_posterior_bench import frame_data

def made(history,cmd,types,tubes,release,rot,crew,selected):
    _,hh,ww=history[-1]
    grid=ALL^sum(B[idx(x)] for x in ww)
    final=flood(sum({B[idx(x)] for x in hh}),grid)
    misses={i for i,p in enumerate(history[-1][0]) if final&B[idx(p)]}
    coords=lambda p: (p%30,p//30) if rot else divmod(p,30)
    kept=[]
    for t in range(101,release):
        if t+1>=300:break
        ps,hs,ws=history[t]
        pn,hn,wn=history[t+1]
        p=list(map(idx,ps));h=list(map(idx,hs));after=list(map(idx,hn))
        w=set(map(idx,ws));next_w=set(map(idx,wn))
        pet_mask=sum({B[q] for q in p});pblocked=pet_mask|wave(pet_mask)
        hblocked=sum({B[q] for q in h})
        already=set()
        tables=[]
        for k,x in enumerate(tubes):
            b,r,l,g,j,rh,*z=x;body=set(z[:4]);side=set(z[4:])
            if not side<=w or ({g,j}|body)&next_w or body&set(h):continue
            gb=B[g]|B[j]
            ready=bool(body&set(p)) and not gb&(pblocked|hblocked)
            if ready and l in h and rh in h:
                already|={i for i,q in enumerate(h) if q in (l,rh)}
            tables.append((k,x,ready))
        for k,x,ready in tables:
            if ready:continue
            b,r,l,g,j,rh,*z=x;body=set(z[:4]);side=set(z[4:])
            if not side<=next_w or any(q in body and i not in (crew[r>=14][b],crew[r>=14][b+1])
                    for i,q in enumerate(after)):continue
            a,b=crew[r>=14][b],crew[r>=14][b+1]
            if a==b or {a,b}&already or any(cmd[t][i].islower() for i in (a,b)):continue
            ds=[]
            for v,dest in ((a,l),(b,rh)):
                ra,ca=coords(h[v]);rb,cb=coords(dest)
                if ca!=cb or dest in next_w:ds.append(999)
                else:ds.append(abs(ra-rb))
            if max(ds)>1 or min(ds)>0:continue
            changed=int(after[a]!=l or after[b]!=rh)
            selected_other=sorted({g for g,workers in selected[t] if g!=k and set(workers)&{a,b}})
            for dog,pos in enumerate(p):
                if types[dog]!=4 or pos not in body|{l,g,j,rh}:continue
                own=idx(pn[dog]);safe=own in (z[1],z[2])
                other=sum({B[idx(v)] for i,v in enumerate(pn) if i!=dog})
                blocker=bool((B[g]|B[j])&(other|wave(other)))
                hnext=set(after)
                factual_legal=int(safe and not blocker and not ({g,j}|body)&hnext)
                kept.append(dict(turn=t,dog=dog,tube=k,ga=a,gb=b,need=max(ds),changed=changed,release=release,
                                 selected_other=selected_other,observed_dog_safe=int(safe),
                                 observed_other_pet_blocks=int(safe and blocker),observed_all_pet_legal=factual_legal,
                                 baseline_final_unresolved=int(dog in misses)))
    return kept

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','observer','probe','inputs','control','out','summary'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark import replay
    from benchmark_cases import run
    with a.control.open() as f:ref=[r for r in csv.DictReader(f) if r['model']=='narrow']
    rows=[]
    for r in ref:
        name=r['case'];case=(a.inputs/name).read_bytes();release=int(r['release'])
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);meta=d/'geom';zones=d/'zones';chosen=d/'chosen';act=d/'act';trace=d/'trace'
            test=a.tester.resolve()
            cmd=[test,'env',f'AHC008_BANK_LOG={meta}',f'AHC008_ZONE_AUDIT={zones}',
                 f'AHC008_ASSIGNED_AUDIT={chosen}',a.observer.resolve()]
            stdout,e,rc,_=run(cmd,case,3.)
            assert rc==0 and e.strip()==('Score = '+r['score']).encode()
            assert r['actions_sha256']==hashlib.sha256(stdout).hexdigest()
            act.write_bytes(stdout)
            pb,e2,rc2,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py','--playback',act,trace],case,10.)
            assert (pb,e2,rc2)==(stdout,e,0)
            lines=trace.read_text().splitlines();actions=stdout.decode().splitlines()
            assert replay(case.decode(),lines,actions,detailed=True)[0]==int(r['score'])
            kinds,hist,_=observed(case,lines)
            geotxt=meta.read_text().splitlines();rot=int(geotxt[0].split()[0])
            tubes=[list(map(int,x.split())) for x in geotxt[1:]]
            if 4 not in kinds:continue
            active,mask,*work=map(int,zones.read_text().split());assert active
            per=len(work)//2;crew=[work[:per],work[per:]]
            prior=defaultdict(list)
            for v in chosen.read_text().splitlines() if chosen.exists() else []:
                t,j,l,h=map(int,v.split());prior[t].append((j,[l,h]))
            queries=made(hist,actions,kinds,tubes,release,rot,crew,prior)
            if not queries:continue
            args=frame_data(hist,actions,lines,kinds,tubes,release)+str(len(queries))+'\n'
            for k,v in enumerate(queries):
                cmd=[v['turn'],k,v['dog'],v['tube'],v['ga'],v['gb']]
                args+=' '.join(map(str,cmd))+'\n'
            pred=subprocess.run([str(a.probe.resolve()),'--aligned'],text=True,input=args,
                                capture_output=True,check=True)
            found=set()
            for line in pred.stdout.splitlines():
                if not line.startswith('A '):continue
                _,k,u,v,w=line.split();k=int(k)
                q=queries[k];q.update(case=name,hold_post=float(u),hold_reset=float(v),factual_post=float(w))
                found.add(k)
            assert len(found)==len(queries)
            rows.extend(queries)
    info={}
    for name,pred in [('all',lambda q:True),('changed',lambda q:q['changed']),
                      ('high',lambda q:q['changed'] and q['hold_post']>=.5),
                      ('high_no_other_assignment',lambda q:q['changed'] and q['hold_post']>=.5 and not q['selected_other'])]:
        qq=[r for r in rows if pred(r)]
        novel=[r for r in qq if r['observed_all_pet_legal'] and r['baseline_final_unresolved']]
        info[name]=dict(rows=len(qq),cases=len({r['case'] for r in qq}),hidden_other_pair=sum(bool(r['selected_other']) for r in qq),
                        held_shift_mean=sum(abs(r['hold_post']-r['factual_post']) for r in qq)/len(qq) if qq else 0,
                        observed_safe=sum(r['observed_dog_safe'] for r in qq),
                        other_pet_blocks=sum(r['observed_other_pet_blocks'] for r in qq),
                        observed_all_pet_legal=sum(r['observed_all_pet_legal'] for r in qq),
                        distinct_missed=len({(r['case'],r['dog']) for r in novel}),
                        missed_cases=len({r['case'] for r in novel}))
    a.out.write_text(''.join(json.dumps(r,separators=(',',':'))+'\n' for r in rows))
    s=dict(verified=len(ref),subsets=info)
    a.summary.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))
if __name__=='__main__':main()
