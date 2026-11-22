"""Observed-only depth<=2 standby-reserve substitution ceilings on frozen banks."""
import argparse,csv,hashlib,json,sys,tempfile
from pathlib import Path
from guarded_entry_trace import observed,neigh,step,STEPS
from stochastic_lock_audit import B,ALL,idx,wave,flood

def audit(case, hist, actions, geom, release, roster, reserves):
    grid=[ALL^sum(B[idx(q)] for q in z[2]) for z in hist]
    dogs,hs,_=hist[-1]
    final=flood(sum({B[idx(h)] for h in hs}),grid[-1])
    misses={i for i,p in enumerate(dogs) if final&B[idx(p)]}
    tubes=[]
    rot,_,_=map(int,geom[0].split())
    for s in geom[1:]:
        b,r,l,g,k,h,*v=map(int,s.split())
        body=sum(B[p] for p in v[:4]);brace=sum(B[p] for p in v[4:])
        assert wave(body)&~body==brace|B[g]|B[k]
        tubes.append((b,r,(l,h),(g,k),body,brace))
    windows={}
    for t in range(101,release):
        pets,humans,_=hist[t]
        humans_bit=sum({B[idx(p)] for p in humans})
        pet_bit=sum({B[idx(p)] for p in pets})
        forbid=humans_bit|pet_bit|wave(pet_bit)
        open_=grid[t]
        for j,(b,r,stances,gates,body,brace) in enumerate(tubes):
            if open_&brace or open_&body!=body or not body&pet_bit or body&humans_bit:continue
            gatebits=B[gates[0]]|B[gates[1]]
            if open_&gatebits!=gatebits or gatebits&forbid:continue
            crew=(roster[int(r>=14)][b],roster[int(r>=14)][b+1])
            staffed=tuple(idx(humans[w])==p for w,p in zip(crew,stances))
            if sum(staffed)!=1:continue
            occupied=0 if staffed[0] else 1
            other=stances[1-occupied]
            if any(idx(h)==other and j not in reserves for j,h in enumerate(humans)):continue
            partner=crew[occupied]
            petids={i for i,p in enumerate(pets) if body&B[idx(p)]}
            windows[t,j,occupied,partner]=petids
    # Do not claim that pet trajectories would stay the same under the
    # hypothetical approach; future observations only cap feasibility.
    opportunities=[]
    nominal=set();nominal_idle=set();base_missed=set()
    for (t,j,side,partner),ids in windows.items():
        base_missed.update(ids&misses)
        goal=tubes[j][2][1-side]
        for reserve in reserves:
            origin=idx(hist[t][1][reserve]);mask=B[origin]
            for d in range(3):
                if mask&B[goal]:
                    nominal.add((t,j))
                    if actions[t][reserve]=='.':nominal_idle.add((t,j))
                    break
                mask|=wave(mask)&grid[t]
    observed_summary=dict(one_normal_windows=len(windows),
        one_normal_unresolved_ids=len(base_missed),
        near_two_ignoring_future=len(nominal),near_two_and_free_now=len(nominal_idle))
    def endpoint_ok(u,tube,side,partner,reserve,job_policy):
        _,_,st,g,body,_=tubes[tube]
        a=actions[u][partner]
        if a.islower():
            p=hist[u][1][partner]
            if idx(step(p,STEPS[a.upper()]))!=g[side]:return False
        if job_policy=='strict' and a in STEPS:return False
        next_open=grid[u+1]&~(B[g[0]]|B[g[1]])
        projected=[idx(p) for p in hist[u+1][1]]
        projected[partner]=st[side];projected[reserve]=st[1-side]
        h=sum({B[p] for p in projected})
        if body&next_open!=body or h&next_open!=h:return False
        if flood(h&-h,next_open)&h!=h:return False
        assert flood(body&-body,next_open)==body
        return True
    for (t,j,side,partner),petids in windows.items():
        _,_,st,_,_,_=tubes[j]; goal=st[1-side]
        for reserve in reserves:
            start=hist[t][1][reserve]; places={start}
            cross=(start[0] if rot else start[1]) != (goal//30 if rot else goal%30)
            alive=set(petids)
            for d in range(3):
                u=t+d
                if u>=release or actions[u][reserve]!='.':break
                alive&=windows.get((u,j,side,partner),set())
                if not alive:break
                if any(idx(x)==goal for x in places):
                    for tag in ('wall_preserving','strict'):
                        if endpoint_ok(u,j,side,partner,reserve,tag):
                            for pet in sorted(alive):
                                opportunities.append(dict(case=case,turn=t,closure_turn=u,eta=d,tube=j,
                                      partner=partner,reserve=reserve,policy=tag,pet=pet,
                                      final_missed=int(pet in misses),cross_column=int(cross)))
                places={q for p in places if grid[u+1]&B[idx(p)]
                        for q in [p,*neigh(p)] if grid[u+1]&B[idx(q)]}
    return observed_summary,opportunities

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','instrumented','cases','control','output','json'):
        p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with a.control.open() as f:
        ref={r['case']:r for r in csv.DictReader(f) if r['model']=='narrow'}
    case_rows=[]; out=[]
    for f in sorted(a.cases.glob('*.txt'))[:30]:
        r=ref[f.name];m=int(r['workers'])
        if m==8:continue
        assert m in (7,9,10)
        with tempfile.TemporaryDirectory() as d:
            temp=Path(d); z=temp/'zones'; b=temp/'banks'; stage=temp/'release'
            tools=[a.tester.resolve(),'env',f'AHC008_ZONE_AUDIT={z}',f'AHC008_BANK_LOG={b}',
                   f'AHC008_RESCUE_LOG={stage}',a.instrumented.resolve()]
            acts,err,rc,_=run(tools,f.read_bytes(),3.)
            assert rc==0 and err.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(acts).hexdigest()==r['actions_sha256']
            act=temp/'act'; act.write_bytes(acts);log=temp/'trace'
            play,err2,rc2,_=run([a.tester.resolve(),sys.executable,a.harness.resolve()/'benchmark_cases.py',
                                '--playback',act,log],f.read_bytes(),10.)
            assert (play,err2,rc2)==(acts,err,0)
            txt=log.read_text().splitlines();actions=acts.decode().splitlines()
            assert int(r['score'])==replay(f.read_text(),txt,actions,detailed=True)[0]
            sp,hist,_=observed(f.read_bytes(),txt)
            active,mask,*workers=map(int,z.read_text().split())
            assert active and len(workers)==8 and set(workers)=={w for w in range(m) if mask>>w&1}
            reserves={w for w in range(m) if not (mask>>w&1)}
            assert len(reserves)==(2 if m==10 else 1)
            _,release=map(int,stage.read_text().split())
            stats, rows=audit(f.name,hist,actions,b.read_text().splitlines(),release,[workers[:4],workers[4:]],reserves)
            for item in rows:item['species']=sp[item['pet']]
            case_rows.append(dict(case=f.name,m=m,release=release,reserves=sorted(reserves),**stats))
            out+=rows
    fields=['case','turn','closure_turn','eta','tube','partner','reserve','policy','pet','final_missed','cross_column','species']
    with a.output.open('w') as f:
        wr=csv.DictWriter(f,fields,lineterminator='\n');wr.writeheader();wr.writerows(out)
    results={}
    for tag in ('wall_preserving','strict'):
        for d in range(3):
            rows=[r for r in out if r['policy']==tag and r['eta']<=d]
            novel=[r for r in rows if r['final_missed']]
            results[tag+str(d)]=dict(windows=len({(r['case'],r['turn'],r['tube']) for r in rows}),
                 total_unique=len({(r['case'],r['pet']) for r in rows}),
                 total_cases=len({r['case'] for r in rows}),
                 missed_unique=len({(r['case'],r['pet']) for r in novel}),
                 missed_cases=len({r['case'] for r in novel}),
                 cross_missing=len({(r['case'],r['pet']) for r in novel if r['cross_column']}))
    a.json.write_text(json.dumps(dict(cases=case_rows,results=results),indent=2)+'\n')
    print(json.dumps(results,indent=2))

if __name__=='__main__':main()
