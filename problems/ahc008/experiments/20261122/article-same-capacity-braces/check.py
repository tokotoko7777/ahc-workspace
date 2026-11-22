"""Read-only simultaneous same-actor brace remapping in exact v7 traces."""
import argparse,csv,hashlib,json,sys,tempfile,collections
from pathlib import Path
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import ALL,B,idx,flood,wave
from parallel_banks_anchored_cut_audit import buf,bits

def examine(name,tubes,hist,cmds):
    s=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in hist]
    starter={}
    for t,(_,_,w) in enumerate(s):
        by=collections.Counter(x.bank for x in tubes if x.wall&w==x.wall and not (x.gates|x.body)&w)
        for i,v in by.items():
            if v>=5 and i not in starter:starter[i]=t
    braces=buf(a for x in tubes for a in bits(x.wall))
    rows=[];examined=0
    for t in range(76):
        p,h,w=s[t];_,after,wa=s[t+1]
        fixed=set()
        actors=[];actual=[]
        for i,c in enumerate(cmds[t]):
            if not c.islower():continue
            q=idx(step(divmod(h[i],30),STEPS[c.upper()]))
            if B[q]&braces:
                actors.append(i);actual.append(q)
            else:fixed.add(q)
        if not actors:continue
        f=buf(fixed)
        # Holds and paid captures/moves outside these actual builders are
        # fixed; in particular no target can be a real movement endpoint.
        pp=buf(p);hh=buf(h);posthum=buf(after)
        blocked=pp|wave(pp)|hh|posthum|w|f
        assert all(not B[q]&blocked for q in actual)
        available=[list(bits(wave(B[h[i]])&braces&~blocked)) for i in actors]
        assert all(q in ops for q,ops in zip(actual,available,strict=True))
        base=w|buf(actual)|f
        assert base==wa
        examined+=1
        if all(len(op)==1 for op in available):continue
        p1=buf(s[t+1][0])
        banks=1+max(x.bank for x in tubes)
        def measure(mask):
            rd=[0]*banks;staff=0;legal_pre=0
            for x in tubes:
                if x.wall&mask!=x.wall or (x.gates|x.body)&mask:continue
                rd[x.bank]+=1
                if x.l in after and x.r in after and not x.body&posthum:
                    staff+=1
                    if x.body&pp and not x.gates&(pp|wave(pp)|posthum):legal_pre+=1
            return (sum(rd),sum(x>=5 for x in rd),staff,legal_pre),rd
        b,bc=measure(base)
        old_parts=[]
        rem=posthum
        while rem:
            cr=flood(rem&-rem,ALL^base)
            old_parts.append(cr&rem);rem&=~cr
        def valid(mask):
            return all(flood(cr&-cr,ALL^mask)&cr==cr for cr in old_parts)
        chosen=actual
        best=b
        unique=set();candidate_info=[]
        first={}
        staff_plus=0
        def dfs(j,mask,path):
            nonlocal chosen,best,staff_plus
            if j==len(available):
                if mask in unique:return
                unique.add(mask)
                whole=w|f|mask
                z,a=measure(whole)
                if z[0]<b[0] or not valid(whole):return
                if z>best:best=z;chosen=path[:]
                if z[2]>b[2]:staff_plus=max(staff_plus,z[2]-b[2])
                for i,(c,new) in enumerate(zip(bc,a)):
                    if c<5<=new and t+1<starter.get(i,301):
                        prev=first.get(i)
                        if prev is None or z>prev[0]:
                            first[i]=(z,path[:])
                return
            for p in available[j]:
                if not mask&B[p]:dfs(j+1,mask|B[p],path+[p])
        dfs(0,0,[])
        rows.append(dict(case=name,turn=t,builder_ids=actors,old=actual,new=chosen,
                         old_metric=b,metric=best,alternate_masks=len(unique)-1,
                         first5=[dict(bank=i,known_actual_first=starter.get(i),moves=path,
                                      metric=z) for i,(z,path) in sorted(first.items())],
                         staff_gain=staff_plus))
    return rows,examined

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','bridge_dir','tester','solver','cases','reference','out','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path[:0]=[str(a.harness.resolve()),str(a.bridge_dir.resolve())]
    from benchmark_cases import run
    from benchmark import replay
    from bridge import parse_geometry
    with open(a.reference) as f: refs=[r for r in csv.DictReader(f) if r['model']=='article']
    rows=[];turns=0
    for r in refs:
        name=r['case'];data=(a.cases/name).read_bytes();test=a.tester.resolve()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);g=d/'geo';tr=d/'tape';act=d/'act'
            o,e,rc,_=run([test,'env',f'AHC008_BANK_LOG={g}',a.solver.resolve()],data,3.)
            assert not rc and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tr],data,10.)
            assert (po,pe,pr)==(o,e,0)
            tape=tr.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(data.decode(),tape,cmds,detailed=True)[0]==int(r['score'])
            _,history,_=observed(data,tape)
            _,_,tubes=parse_geometry(g)
            z,n=examine(name,tubes,history,cmds);rows+=z;turns+=n
        print(name,flush=True)
    improve=[r for r in rows if r['metric'][0]>r['old_metric'][0]]
    a5=[(r['case'],r['turn'],z) for r in rows for z in r['first5']]
    astaff=[r for r in rows if r['staff_gain']]
    cases=lambda rr:len({r['case'] for r in rr})
    s=dict(verified=len(refs),ordinary_brace_turns=turns,turns_with_actor_choice=len(rows),
           alternatives=sum(r['alternate_masks'] for r in rows),
           improving_turns=len(improve),improving_cases=cases(improve),max_delta=sum(
               r['metric'][0]-r['old_metric'][0] for r in improve),
           first5_gains=len(a5),first5_cases=len({n for n,t,v in a5}),
           first5_distinct_banks=len({(n,v['bank']) for n,t,v in a5}),
           ready_staff_gains=len(astaff),ready_staff_cases=cases(astaff),
           max_legal_pre_gains=sum(r['metric'][3]>r['old_metric'][3] for r in rows))
    a.out.write_text(''.join(json.dumps(r,separators=(',',':'))+'\n' for r in rows))
    a.summary.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))
if __name__=='__main__':main()
