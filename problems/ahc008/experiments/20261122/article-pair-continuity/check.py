"""Selected real v7 short bank pair continuity: unchanged-output trace audit."""
import argparse,csv,hashlib,json,sys,tempfile,collections
from pathlib import Path
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import ALL,B,idx,flood,wave
from parallel_banks_anchored_cut_audit import buf

def verify_case(name,log,bridges,geometry,history,cmds,meta):
    from bridge import parse_geometry,nav,recover
    _,_,tubes=parse_geometry(geometry)
    active={int(z.split()[0]) for z in meta.split(';') if z and int(z.split()[1]) in (1,2)}
    s=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in history]
    final=flood(buf(s[-1][1]),ALL^s[-1][2])
    missed={i for i,p in enumerate(s[-1][0]) if B[p]&final}
    pair=collections.defaultdict(list)
    paid=collections.defaultdict(dict)
    routed=collections.defaultdict(set)
    for z in bridges.read_text().splitlines():
        t,k,i,u,*_=map(int,z.split())
        if k in (3,4):routed[t].add(i)
    def walljob(t,i):
        c=cmds[t][i]
        if not c.islower():return None
        return idx(step(divmod(s[t][1][i],30),STEPS[c.upper()]))
    for line in log.read_text().splitlines():
        k,*r=line.split()
        t,u,a,b,*tail=map(int,r)
        if k=='P':
            pair[t].append((u,a,b,*tail))
        else:
            paid[t][a]=tubes[u].g
            paid[t][b]=tubes[u].z
            assert walljob(t,a)==tubes[u].g and walljob(t,b)==tubes[u].z
    out=[]
    for t,ts in pair.items():
        for k,a,b,h,al,bl,at,bt in ts:
            if h>3:continue
            x=tubes[k];u=t+h
            assert h==max(al,bl,1) and t in active
            p,hh,w=s[t];body=buf(q for y in tubes if y.wall&w==y.wall for q in y.inside)
            assert x.wall&w==x.wall and not x.gates&w
            op=ALL^(w|buf(paid[t].values()))
            paths=[]
            for j,goal,d,first in [(a,x.l,al,at),(b,x.r,bl,bt)]:
                src=hh[j]
                if d==0:
                    assert goal==src and first<0
                    path=[src]
                else:
                    own=next((y.body for y in tubes if y.body&B[src] and y.wall&w==y.wall),0)
                    allowed=op&(~body|own)
                    dd=nav(goal,allowed)
                    assert dd[src]==d and dd[first]==d-1
                    path=[src]+recover(first,dd)
                paths.append(path+[goal]*(h+1-len(path)))
            crew=[a,b]
            result=dict(case=name,dispatch=t,tube=k,eta=h,crew=crew,path=paths)
            cancelled=sum(not any(y[:3]==(k,a,b) for y in pair.get(z,())) for z in range(t+1,u))
            result['announced_continuation_missing']=cancelled
            arrived=s[u][1][a]==x.l and s[u][1][b]==x.r
            phys_different=any(s[t+j][1][i]!=path[j] for i,path in zip(crew,paths) for j in range(h+1))
            result['physical_path_changed']=phys_different
            result['arrived']=arrived
            if any(s[f][2]&x.gates==x.gates for f in range(t+1,u+1)):
                out.append(dict(result,status='already_sealed'))
                continue
            if u not in active or any(z not in active for z in range(t,u+1)):
                out.append(dict(result,status='phase_exit'))
                continue
            if any(B[q]&s[t+j][2] for path in paths for j,q in enumerate(path)):
                out.append(dict(result,status='physical_block'))
                continue
            if arrived:
                out.append(dict(result,status='arrived'))
                continue
            if not phys_different:raise AssertionError('canonical path reaches assigned stance')
            pp,hu,wu=s[u];_,after,wn=s[u+1]
            future=wu
            block=buf(pp)
            before=list(hu);after=list(after)
            for i,q in [(a,x.l),(b,x.r)]:before[i]=after[i]=q
            legal=not (x.gates|x.body)&wu and x.wall&wu==x.wall and not (x.gates&(block|wave(block)|buf(before)))
            # Preserve all issued actions of the other actors at final time.
            others=[walljob(u,j) for j in range(len(hh)) if j not in crew and walljob(u,j) is not None]
            blocked=block|wave(block)|buf(before)
            if any(B[q]&(blocked|wu|x.gates) for q in others):legal=False
            future|=buf(others)|x.gates
            if buf(after)&(future|x.body):legal=False
            if legal:
                all_hum=flood(B[after[0]],ALL^future)
                legal=buf(after)&all_hum==buf(after)
            ids=[i for i,q in enumerate(pp) if q in x.inside[1:3]] if legal else []
            if ids:
                assert flood(B[pp[ids[0]]],ALL^future)==x.body and not x.body&all_hum
            other_paired=set();displaced_gates=displaced_braces=displaced_bridge=0
            for ch,path,own_gate in zip(crew,paths,[x.g,x.z]):
                for z in range(t+1,u+1):
                    wj=walljob(z,ch)
                    if wj is not None and (z!=u or wj!=own_gate):
                        if ch in paid[z]:displaced_gates+=1
                        else:displaced_braces+=1
                    if ch in routed[z] and (z==u or s[z+1][1][ch]!=path[z+1-t]):displaced_bridge+=1
                    for pu,pa,pb,*pd in pair.get(z,()):
                        if ch in (pa,pb) and (pu,pa,pb)!=(k,a,b) and (z==u or s[z+1][1][ch]!=path[z+1-t]):
                            other_paired.add((z,pu))
            result.update(status='avoidable_path_change',legal_window=ids,otherwise_final_missed=[i for i in ids if i in missed],
                          displaced_gates=displaced_gates,displaced_braces=displaced_braces,
                          displaced_bridge=displaced_bridge,paid_other_pairs=len(other_paired))
            out.append(result)
    return out

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','bridge_dir','tester','solver','cases','reference','out','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path[:0]=[str(a.harness.resolve()),str(a.bridge_dir.resolve())]
    from benchmark_cases import run
    from benchmark import replay
    with open(a.reference) as f:refs=[r for r in csv.DictReader(f) if r['model']=='article']
    out=[]
    for r in refs:
        name=r['case'];c=(a.cases/name).read_bytes();test=a.tester.resolve()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);g=d/'geo';l=d/'log';b=d/'bridges';t=d/'tape';act=d/'act'
            o,e,rc,_=run([test,'env',f'AHC008_PAIR_COMMIT_LOG={l}',f'AHC008_BANK_LOG={g}',f'AHC008_BRIDGE_LOG={b}',a.solver.resolve()],c,3.)
            assert not rc and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,t],c,10.)
            assert (po,pe,pr)==(o,e,0)
            lines=t.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(c.decode(),lines,cmds,detailed=True)[0]==int(r['score'])
            _,hist,_=observed(c,lines)
            out+=verify_case(name,l,b,g,hist,cmds,r['telemetry'])
        print(name,flush=True)
    q=[r for r in out if r.get('legal_window')]
    m=[r for r in q if r['otherwise_final_missed']]
    u=[r for r in m if not any(r[k] for k in ('displaced_gates','displaced_braces','displaced_bridge'))]
    dist=lambda rows:len({(r['case'],i) for r in rows for i in r['otherwise_final_missed']})
    c=lambda rows:len({r['case'] for r in rows})
    s=dict(verified=len(refs),announced=len(out),status=collections.Counter(r['status'] for r in out),
           factual_legal=len(q),factual_legal_cases=c(q),otherwise_final_missed=dist(m),
           otherwise_final_missed_cases=c(m),otherwise_final_missed_no_paid_wall_or_bridge=dist(u),
           otherwise_final_missed_no_paid_wall_or_bridge_cases=c(u),
           no_other_paired_displacement=dist([r for r in u if not r['paid_other_pairs']]))
    a.out.write_text(''.join(json.dumps(r,separators=(',',':'))+'\n' for r in out))
    a.summary.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))
if __name__=='__main__':main()
