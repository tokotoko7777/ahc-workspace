"""Two named retrospective AHC008 cases: exact comparative causal wall/route timelines."""
import argparse,csv,gzip,hashlib,json,sys,tempfile
from pathlib import Path
from guarded_entry_trace import observed,step,STEPS
from stochastic_lock_audit import ALL,B,idx,flood

def occupied(grid,h,pets):
    return [i for i,p in enumerate(pets) if grid&B[idx(p)]], [i for i,p in enumerate(h) if grid&B[idx(p)]]

def snapshot(pets,humans,walls):
    a=ALL^sum(B[idx(q)] for q in walls)
    ans=[]
    for i,h in enumerate(humans):
        c=flood(B[idx(h)],a)
        pp,hh=occupied(c,humans,pets)
        ans.append((c.bit_count(),pp,hh))
    return ans

def diagnose(pets, humans, walls, actions, newwalls):
    post=[step(q,STEPS[c]) if c in STEPS else q for c,q in zip(actions,humans)]
    a=ALL^sum(B[idx(q)] for q in walls)
    az=ALL^sum(B[idx(q)] for q in newwalls)
    before=snapshot(pets,humans,walls)
    moves_only=snapshot(pets,post,walls)
    after=snapshot(pets,post,newwalls)
    matter=[]
    seen=set()
    for i,p in enumerate(humans):
        c=flood(B[idx(p)],a)
        if c in seen:continue
        seen.add(c)
        pp,hh=occupied(c,humans,pets)
        remaining=c&az
        if not (a^az)&c: continue
        subdivided=[]
        while remaining:
            u=flood(remaining&-remaining,remaining)
            up,uh=occupied(u,post,pets)
            remaining&=~u
            subdivided.append((u.bit_count(),up,uh))
        if len(subdivided)>1:
            matter.append(dict(prior=(c.bit_count(),pp,hh), parts=sorted(subdivided,reverse=True)))
    return before,moves_only,after,post,matter

def audit(hist,cmd,release):
    info={'ticks':[],'all_splits':[],'epochs':{}}
    epochs={0,100,159,160,175,185,200,210,225,240,260,280,300}
    if release is not None:epochs.update({release,release+1})
    for t in sorted(epochs):
        p,h,w=hist[t]; info['epochs'][t]=snapshot(p,h,w)
    for t in range(300):
        ps,hs,ws=hist[t]
        _,_,wn=hist[t+1]
        a=cmd[t]
        row=dict(t=t,actions=a,humans_before=hs,pet_before=ps, walls_added=sorted(wn-ws))
        if wn!=ws:
            before,move,after,h,matter=diagnose(ps,hs,ws,a,wn)
            row.update(humans_after=h, before=before, after_move_only=move, after_actual_cut=after)
            if matter:info['all_splits'].append({**row,'cuts':matter})
        info['ticks'].append(row)
    return info

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','stay_rescue','standalone','tactical','cases','reference_gzip','output'):
        p.add_argument(n,type=Path)
    p.add_argument('names',nargs='+')
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with gzip.open(a.reference_gzip,'rt') as f:
        ref={(r['case'],r['model']):r for r in csv.DictReader(f)}
    report={}
    for n in a.names:
        path=a.cases/n; case=path.read_bytes()
        for arm,args in [('stay225',[a.stay_rescue.resolve(),'5','0']),
                         ('rescue160',[a.standalone.resolve()]),('tactical',[a.tactical.resolve()])]:
            with tempfile.TemporaryDirectory() as td:
                q=Path(td)/'meta'; stage=Path(td)/'rescue'
                command=[a.tester.resolve(),'env',f'AHC008_BANK_LOG={q}',f'AHC008_RESCUE_LOG={stage}',*args]
                actions,err,rc,_=run(command,case,3.)
                prior=ref[n,arm];score=int(prior['score'])
                assert rc==0 and err.strip()==f'Score = {score}'.encode()
                assert hashlib.sha256(actions).hexdigest()==prior['actions_sha256']
                blob=Path(td)/'a';trace=Path(td)/'trace';blob.write_bytes(actions)
                play,err2,rc2,_=run([a.tester.resolve(),sys.executable,a.harness.resolve()/'benchmark_cases.py',
                                    '--playback',blob,trace],case,10.)
                assert (play,err2,rc2)==(actions,err,0)
                logs=trace.read_text().splitlines(); cmd=actions.decode().splitlines()
                assert score==replay(case.decode(),logs,cmd,detailed=True)[0]
                species,hist,_=observed(case,logs)
                release_num=int(stage.read_text().split()[1]) if stage.exists() else None
                x=audit(hist,cmd,release_num)
                x.update(score=score,release=release_num,species=species, geometry=q.read_text().splitlines() if q.exists() else [])
                report.setdefault(n,{})[arm]=x
    with a.output.open('w') as f:json.dump(report,f,separators=(',',':'));f.write('\n')

if __name__=='__main__':main()
