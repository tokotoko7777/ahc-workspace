"""Three frozen matched traces: isolated immediate graph counterfactuals."""
import argparse,csv,gzip,hashlib,json,sys,tempfile
from pathlib import Path
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import ALL,B,ADJ,idx,flood
SELECT={'practice':('0005.txt','0009.txt'),'generated':('0059.txt',)}
def xy(p):return (p//30,p%30)
def components(ps,hs,ws):
    available=ALL^sum(B[idx(w)] for w in ws)
    cache={}
    def find(h):
        k=idx(h)
        if k not in cache:
            mask=flood(B[k],available)
            data=(mask.bit_count(),sum(bool(mask&B[idx(p)]) for p in ps))
            for z in hs:
                if mask&B[idx(z)]:cache[idx(z)]=data
        return cache[k]
    return [find(h) for h in hs]
def major(hist):
    out=[]
    for t in range(300):
        ps,h,w=hist[t]
        pn,hn,wn=hist[t+1]
        if w==wn:continue
        before=components(ps,hn,w)
        after=components(ps,hn,wn) # use same humans and pets, isolate wall effect
        losses=[s[0]-d[0] for s,d in zip(before,after)]
        escaped=[s[1]-d[1] for s,d in zip(before,after)]
        if max(losses)>=40 or max(escaped)>=3:
            out.append(dict(t=t,delta_area=losses,delta_reachable=escaped,
                            walls=sorted(wn-w),after=after))
    return out
def parse(log):
    rows={}
    for l in log.splitlines():
        a,*tail=l.split('|')
        head,foot=a.split('@')
        t,prev,cmd,dog,closed,gate,decoy=head.split()
        plans=[list(map(int,z.split())) for z in tail]
        rows[int(t)]=dict(prior=prev,cmd=cmd,dog=int(dog),closed=int(closed),
                          dog_gate=int(gate),dog_decoy=int(decoy),dog_walls=[xy(int(w)) for w in foot.split()],
                          plans=[dict(id=z[0],center=xy(z[1]),radius=z[2],wall=[xy(x) for x in z[3:]]) for z in plans])
    return rows
def omitted(hist,logs,truth_log):
    out=[]
    for t,row in logs.items():
        prior,cmd=row['prior'],row['cmd']
        ps,hs,ws=hist[t]; pn,after,wn=hist[t+1]
        for i,(a,b) in enumerate(zip(prior,cmd)):
            if not a.islower() or a==b:continue
            q=step(hs[i],STEPS[a.upper()])
            ph=list(after); ph[i]=hs[i]
            legal=(q not in set(hs)|set(ps) and q not in ph and not any(idx(p) in ADJ[idx(q)] for p in ps))
            after_real=components(ps,after,wn)
            alt=set(wn);remove=step(hs[i],STEPS[b.upper()]) if b.islower() else None
            # Don't erase a wall preexisting or also requested by another worker.
            if remove is not None and remove not in ws and not any(
                j!=i and c.islower() and step(hs[j],STEPS[c.upper()])==remove for j,c in enumerate(cmd)):
                alt.discard(remove)
            alternative=components(ps,ph,alt|{q}) if legal else None
            out.append(dict(t=t,worker=i,current=hs[i],wall=q,replaced_by=b,legal_same_turn=legal,
                            actual=after_real,alternative=alternative,original_arm_plan=row['plans'],
                            unmodified_control_plan=truth_log[t]['plans'],
                            variant_dog_phase=row['dog'],control_dog_phase=truth_log[t]['dog'],
                            in_intended_dog_fence=q in row['dog_walls']))
    return out
def read_case(bench,tester,exe,inp,harness,tag):
    from benchmark_cases import run
    from benchmark import replay
    with tempfile.TemporaryDirectory() as td:
        lg=Path(td)/'cause'
        quad=Path(td)/'quad'
        if tag=='control':cmd=[tester,'env',f'AHC008_CAUSE_LOG={lg}',exe]
        else:cmd=[tester,'env',f'AHC008_CAUSE_LOG={lg}',f'AHC008_QUAD_LOG={quad}',exe,'2','27','1']
        actions,err,rc,_=run(cmd,inp,3.)
        assert rc==0
        assert hashlib.sha256(actions).hexdigest()==bench['actions_sha256']
        assert int(bench['score'])==int(err.split()[-1])
        a=Path(td)/'a';a.write_bytes(actions);trace=Path(td)/'trace'
        replay_actions,e2,c2,_=run([tester,sys.executable,harness/'benchmark_cases.py','--playback',a,trace],inp,10.)
        assert (replay_actions,e2,c2)==(actions,err,0)
        txt=trace.read_text().splitlines()
        score,*_=replay(inp.decode(),txt,actions.decode().splitlines(),detailed=True)
        assert score==int(bench['score'])
        _,hist,_=observed(inp,txt)
        lgp=parse(lg.read_text())
        phases=[dict(turn=t,phase=z['dog'],closed=z['closed'],decoy=z['dog_decoy'])
                for t,z in lgp.items() if t==0 or (z['dog'],z['closed'])!=(lgp[t-1]['dog'],lgp[t-1]['closed'])]
        planned=lgp[0]['dog_walls']
        gate=lgp[0]['dog_gate']
        gate_turn=next((t for t in range(300) if gate>=0 and xy(gate) in hist[t+1][2]-hist[t][2]),None)
        ready_turn=next((t for t in range(301) if set(planned)<=hist[t][2]),None) if planned else None
        footprint={}
        if quad.exists():
            meta=quad.read_text().splitlines()[0].split()
            if len(meta)>13:
                interior=[xy(int(x)) for x in meta[7:11]]
                gates=[xy(int(x)) for x in meta[13:]]
                footprint=dict(pen=interior,shell=gates,begin_overlap_dog_walls=sorted(set(gates)&set(planned)))
        return dict(quad_footprint=footprint,dog_phases=phases,dog_gate=xy(gate),dog_gate_actual_wall_turn=gate_turn,dog_fence_ready=ready_turn,
                    dog_fence_cells=len(planned),score=score,digest=bench['actions_sha256'],final=components(*hist[-1]),
                    separation=major(hist),hist=hist,logs=lgp)
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','instrumented','practice_cases','generated_cases','archive'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    selected=[]
    for group,cs in SELECT.items():
        fname=a.archive/f'quad-defer-{group}100.csv.gz'
        with gzip.open(fname,'rt') as f: frozen={(r['case'],r['model']):r for r in csv.DictReader(f)}
        for cf in cs:
            inp=(getattr(a,group+'_cases')/cf).read_bytes()
            pair={}
            for arm in ('control','deferred-square'):
                row=frozen[cf,arm]
                assert hashlib.sha256(inp).hexdigest()==row['input_sha256']
                pair[arm]=read_case(row,a.tester.resolve(),a.instrumented.resolve(),inp,a.harness.resolve(),arm)
            baseline,quad=pair['control'],pair['deferred-square']
            history=quad['hist']; cl=baseline['hist']
            first_act=next(t for t in range(300) if quad['logs'][t]['cmd']!=baseline['logs'][t]['cmd'])
            first_w=next(t for t in range(300) if history[t+1][2]!=cl[t+1][2])
            dis=omitted(history,quad['logs'],baseline['logs'])
            early=[x for x in dis if x['t']<68]
            out=dict(group=group,case=cf,first_action_difference=first_act,first_wall_state_difference=first_w,
                     first_different_wall_delayed_control=sorted(cl[first_w+1][2]-history[first_w+1][2]),
                     first_different_wall_in_quad=sorted(history[first_w+1][2]-cl[first_w+1][2]),
                     early_displaced_wall_count=len(early),early_restore_major_40cells=sum(
                        x['alternative'] is not None and any(abs(t[0]-v[0])>=40 for t,v in zip(x['actual'],x['alternative'])) for x in early),
                     early_omissions=early)
            for arm,d in pair.items():out[arm]={k:v for k,v in d.items() if k not in ('hist','logs')}
            selected.append(out)
    print(json.dumps(selected,indent=2))
if __name__=='__main__':main()
