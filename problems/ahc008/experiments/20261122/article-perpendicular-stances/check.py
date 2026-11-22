"""Physical outward/perpendicular existing gate stances in exact v7 bank play."""
import argparse,csv,hashlib,json,sys,tempfile,collections,itertools
from pathlib import Path
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import ADJ,ALL,B,idx,flood,wave
from parallel_banks_anchored_cut_audit import buf,bits

def examine(name,tubes,hist,cmds,meta,bridge,caps):
    rows=[];stats=collections.Counter()
    st=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in hist]
    mode={int(z.split()[0]):int(z.split()[1]) for z in meta.split(';') if z}
    moving=collections.defaultdict(set);reserved=collections.defaultdict(set)
    for q in bridge.read_text().splitlines():
        t,k,i,u,*_=map(int,q.split())
        if k in (3,4):moving[t].add(i);reserved[t].add(u)
    first={}
    for x in caps.split(';'):
        if x:
            t,j,*_=map(int,x.split(':'));first[j]=t
    F=flood(buf(st[-1][1]),ALL^st[-1][2])
    final={i for i,q in enumerate(st[-1][0]) if B[q]&F}
    for t,prefix in mode.items():
        if prefix not in (0,1,2):continue
        p,h,w=st[t];_,after,wn=st[t+1];pall=buf(p)
        ph=buf(h);forbid=ph|pall|wave(pall)
        paid={i:idx(step(divmod(h[i],30),STEPS[c.upper()])) for i,c in enumerate(cmds[t]) if c.islower()}
        active=[x for x in tubes if x.wall&w==x.wall];body=buf(q for x in active for q in x.inside)
        brace=buf(q for x in tubes for q in bits(x.wall))
        gate0={x.k for x in active if x.gates&w==0 and wn&x.gates==x.gates}
        gate_set=buf(q for j in gate0 for q in (tubes[j].g,tubes[j].z))
        for x in active:
            if x.k in reserved[t] or (x.body|x.gates)&w or x.body&ph or x.gates&forbid or not x.body&pall:
                continue
            ids=[i for i,q in enumerate(p) if B[q]&x.body]
            assert all(p[i] in x.inside[1:3] for i in ids)
            stats['legal_prepared_tube_turns']+=1
            if x.k in gate0:continue
            stats['not_chosen']+=1
            common=~(body|w|(wn&~w))
            gate_ops=[[(i,q) for i,q in enumerate(h) if i not in moving[t] and B[q]&common and q in ADJ[g]]
                      for g in (x.g,x.z)]
            opts=[]
            for (a,left),(b,right) in itertools.product(*gate_ops):
                if a==b or (left==x.l and right==x.r):continue
                crew={a,b}
                provisional=[q for i,q in paid.items() if i not in crew]
                if buf(provisional)&x.gates:continue
                full=w|x.gates|buf(provisional)
                nxt=list(after);nxt[a]=left;nxt[b]=right
                hum=buf(nxt)
                if hum&full:continue
                R=flood(B[nxt[0]],ALL^full)
                if hum&R!=hum or R&x.body:continue
                if flood(B[p[ids[0]]],ALL^full)!=x.body:continue
                gone=[paid[i] for i in crew if i in paid and paid[i]!=(x.g if i==a else x.z)]
                opts.append(dict(actors=[a,b],stance=[left,right],axial=[x.l,x.r],
                 lost_walls=len(gone),lost_original_gate=sum(bool(B[q]&gate_set) for q in gone),
                 lost_braces=sum(bool(B[q]&brace) for q in gone),original_moves_changed=sum(h[i]!=after[i] for i in crew),
                 new_retained=R.bit_count(),target=ids,final_missed=[i for i in ids if i in final],
                 earlier=[i for i in ids if not t+1>=first.get(i,301)]))
            if opts:
                q=min(opts,key=lambda q:(q['lost_original_gate'],q['lost_walls'],q['original_moves_changed'],q['actors']))
                q.update(case=name,turn=t,tube=x.k,phase=prefix,config_alternatives=len(opts))
                rows.append(q)
    return rows,stats

def fixtures(Tube):
    row=15;a=6
    body=tuple(row*30+c for c in range(a+2,a+6))
    side=buf(t*30+c for t in (row-1,row+1) for c in range(a+2,a+6))
    x=Tube(0,0,row,row*30+a,row*30+a+1,row*30+a+6,row*30+a+7,body,side)
    h=[x.g-30,x.z+30,660,661,695]
    p=[body[1]]
    assert all(h[i] in ADJ[g] for i,g in enumerate((x.g,x.z)))
    for i in range(2):assert h[i] not in (x.l,x.r) and not x.body&B[h[i]]
    o=ALL^(side|x.gates)
    assert flood(B[p[0]],o)==x.body
    assert buf(h)&flood(B[h[0]],o)==buf(h)
    neighbor_gate=x.g-60
    assert h[0] in ADJ[neighbor_gate]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','bridge_dir','tester','solver','cases','reference','events','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path[:0]=[str(a.harness.resolve()),str(a.bridge_dir.resolve())]
    from benchmark_cases import run
    from benchmark import replay
    from bridge import parse_geometry,Tube
    fixtures(Tube)
    with open(a.reference) as f:refs=[r for r in csv.DictReader(f) if r['model']=='article']
    rows=[];stats=collections.Counter()
    for r in refs:
        name=r['case'];data=(a.cases/name).read_bytes();test=a.tester.resolve()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);geo=d/'geo';tr=d/'tape';b=d/'bridge';act=d/'act'
            o,e,rc,_=run([test,'env',f'AHC008_BANK_LOG={geo}',f'AHC008_BRIDGE_LOG={b}',a.solver.resolve()],data,3.)
            assert not rc and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tr],data,10.)
            assert (po,pe,pr)==(o,e,0)
            tape=tr.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(data.decode(),tape,cmds,detailed=True)[0]==int(r['score'])
            _,hist,_=observed(data,tape);_,_,tubes=parse_geometry(geo)
            q,z=examine(name,tubes,hist,cmds,r['telemetry'],b,r['captures']);rows+=q;stats.update(z)
        print(name,flush=True)
    strict=[x for x in rows if not x['lost_walls']]
    no_gate=[x for x in rows if not x['lost_original_gate']]
    count=lambda rr:dict(witnesses=len(rr),cases=len({x['case'] for x in rr}),
          animals=len({(x['case'],i) for x in rr for i in x['target']}),
          earlier=len({(x['case'],i) for x in rr for i in x['earlier']}),
          final_missed=len({(x['case'],i) for x in rr for i in x['final_missed']}),
          final_missed_cases=len({x['case'] for x in rr if x['final_missed']}))
    r=dict(verified=len(refs),counts=stats,any=count(rows),preserve_all_old_gate=count(no_gate),
           preserve_every_wall=count(strict))
    a.events.write_text(''.join(json.dumps(x,separators=(',',':'))+'\n' for x in rows))
    a.summary.write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r,indent=2))
if __name__=='__main__':main()
