"""Existing captured brackets -> true seven-cell dogleg feasibility."""
import argparse,csv,hashlib,itertools,json,sys,tempfile,collections
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,ADJ,idx,flood,wave
from parallel_banks_anchored_cut_audit import buf,bits,rect,distances

def stencil(x,rot,side):
    v=1 if rot else 30
    gate=x.g if side==0 else x.z
    other=x.z if side==0 else x.g
    stance=x.l if side==0 else x.r
    cells=x.body|B[gate]|B[gate-v]|B[gate+v]
    cut=[other,stance-v,stance,stance+v]
    return cells,cut

def slots(work,cuts,keep):
    K=len(cuts)
    stance=[[q for q in ADJ[w] if B[q]&keep] for w in cuts]
    all_pos={u for v in stance for u in v}
    routes={q:distances(B[q],keep) for q in all_pos}
    dp={0:(0,[])}
    for i,origin in work:
        d=distances(B[origin],keep)
        jobs={0:(0,[])}
        for j,st in enumerate(stance):
            for q in st:
                if d[q]<0:continue
                one=d[q]+1
                if one<=6 and (1<<j not in jobs or one<jobs[1<<j][0]):
                    jobs[1<<j]=(one,[(j,q,d[q])])
                for k,y in enumerate(stance):
                    if j==k:continue
                    for p in y:
                        dd=routes[q][p]
                        cost=one+dd+1
                        mask=(1<<j)|(1<<k)
                        if dd>=0 and cost<=6 and (mask not in jobs or cost<jobs[mask][0]):
                            jobs[mask]=(cost,[(j,q,d[q]),(k,p,one+dd)])
        upd={}
        for m,(time,ass) in dp.items():
            for mm,(cost,path) in jobs.items():
                if m&mm:continue
                tag=m|mm;t=max(time,cost)
                if tag not in upd or upd[tag][0]>t:upd[tag]=(t,ass+([(i,path)] if path else []))
        dp=upd
    return dp.get((1<<K)-1)

def inspect(name,log,tubes,rot,hist,species):
    st=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in hist]
    roster={};full={}
    for line in log.read_text().splitlines():
        t,m,f=map(int,line.split())
        roster[t]=m;full[t]=f
    tb={(x.bank,x.row):x for x in tubes}
    cmax=max(x.bank for x in tubes)
    final=flood(buf(st[-1][1]),ALL^st[-1][2])
    missed={i for i,q in enumerate(st[-1][0]) if B[q]&final}
    out=[];stats=collections.Counter()
    for t,m in roster.items():
        assert 101<=t<160
        p,h,wall=st[t];pet=buf(p);hum=buf(h)
        root=flood(B[h[0]],ALL^wall)
        if hum&root!=hum:continue
        stats['active_turns']+=1
        for b,side in [(0,0),(cmax,1)]:
            for x in tubes:
                if x.bank!=b or (x.gates|x.body)&wall or not x.wall&wall==x.wall:continue
                y=tb.get((b,x.row-2));z=tb.get((b,x.row+2))
                if y is None or z is None:continue
                if any(q.gates&wall!=q.gates or q.wall&wall!=q.wall or not q.body&pet for q in (y,z)):continue
                pen,co=stencil(x,rot,side)
                mask=buf(co)
                if pen&wall:continue
                keep=flood(B[h[0]],ALL^(wall|mask))
                if hum&keep!=hum or pen&hum:continue
                body=flood(pen&-pen,ALL^(wall|mask))
                if body!=pen or rect(body):continue
                pending=list(q for q in co if not wall&B[q])
                if not pending or root.bit_count()-keep.bit_count()!=7+len(pending):continue
                if st[t+1][2]&x.gates==x.gates:continue # ordinary exact4 already paid
                stats['brackets']+=1
                stats['brackets_full']+=bool(full[t])
                targets=[i for i,q in enumerate(p) if species[i]<=3 and B[q]&pen]
                if not targets:continue
                stats['occupied_passive']+=1
                un=[(i,pp) for i,pp in enumerate(h) if m>>i&1]
                row=dict(case=name,turn=t,tube=x.k,side=side,full=full[t],targets=targets,
                         final_missed=[i for i in targets if i in missed],pen=list(bits(pen)),
                         future_walls=pending,idle=[i for i,_ in un],
                         current_blocked=sum(bool(B[q]&(pet|wave(pet)|hum)) for q in pending))
                out.append(row)
                if len(pending)>2*len(un):continue
                route=slots(un,pending,keep)
                if not route:continue
                eta,paths=route
                row.update(route_time=eta,route=paths)
                close=t+eta-1
                crews={i for i,_ in paths}
                jobs=[(close if j==len(path)-1 else t+s,pending[k],i,q)
                      for i,path in paths for j,(k,q,s) in enumerate(path)]
                row['factual_resident']=[i for i in targets if B[st[close][0][i]]&pen]
                row['factual_walls_legal']=all(not B[q]&(buf(st[z][0])|wave(buf(st[z][0])))
                                               for z,q,i,pos in jobs)
                row['preserves_future_paid']=all(roster.get(z,0)&(1<<i) for z in range(t,close+1) for i in crews)
                actual_other_walls=st[t+1][2]&~wall
                old_h=list(st[t+1][1])
                for i in crews:old_h[i]=h[i]
                row['immediate']=eta==1 and row['current_blocked']==0 and not mask&actual_other_walls and not pen&buf(old_h)
    return out,stats

def selfcheck(Tube):
    def T(r):
        a=15;b=0
        body=tuple(r*30+c for c in range(a+2,a+6));side=buf(s*30+c for s in (r-1,r+1) for c in range(a+2,a+6))
        return Tube(0,b,r,r*30+a,r*30+a+1,r*30+a+6,r*30+a+7,body,side)
    for direction in (0,1):
        x,y,z=map(T,(15,13,17));w=x.wall|y.wall|z.wall|y.gates|z.gates
        p,cut=stencil(x,0,direction)
        assert p.bit_count()==7 and not w&p and flood(p&-p,ALL^(w|buf(cut)))==p
        h=3
        f=flood(B[h],ALL^w)
        keep=flood(B[h],ALL^(w|buf(cut)))
        assert f.bit_count()-keep.bit_count()==11

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
    rr=[];counts=collections.Counter();active=set()
    for r in refs:
        name=r['case'];data=(a.cases/name).read_bytes();test=a.tester.resolve()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);log=d/'diag';geo=d/'geo';tr=d/'tape';act=d/'act'
            o,e,rc,_=run([test,'env',f'AHC008_DOGLEG_LOG={log}',f'AHC008_BANK_LOG={geo}',a.solver.resolve()],data,3.)
            assert not rc and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tr],data,10.)
            assert (po,pe,pr)==(o,e,0)
            tape=tr.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(data.decode(),tape,cmds,detailed=True)[0]==int(r['score'])
            species,hist,_=observed(data,tape);rot,_,tubes=parse_geometry(geo)
            l,c=inspect(name,log,tubes,rot,hist,species);rr+=l;counts.update(c)
            if c['brackets']:active.add(name)
        print(name,flush=True)
    stages=[r for r in rr if 'route' in r]
    feasible=[r for r in stages if r['factual_resident'] and r['factual_walls_legal'] and r['preserves_future_paid']]
    z=dict(verified=len(refs),counts=counts,bracket_cases=len(active),occupied_cases=len({r['case'] for r in rr}),
           occupied_full=sum(r['full'] for r in rr),route_six=len(stages),route_six_cases=len({r['case'] for r in stages}),
           legal_exact_immediate=sum(r['immediate'] for r in stages),factual_both=len(feasible),
           distinct_otherwise_final_missed_feasible=len({(r['case'],i) for r in feasible
                 for i in r['final_missed'] if i in r['factual_resident']}))
    a.out.write_text(''.join(json.dumps(x,separators=(',',':'))+'\n' for x in rr))
    a.summary.write_text(json.dumps(z,indent=2)+'\n');print(json.dumps(z,indent=2))
if __name__=='__main__':main()
