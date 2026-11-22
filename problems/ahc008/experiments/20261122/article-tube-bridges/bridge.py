"""Read-only one-empty-tube bridge audit on fixed official v4 dev tapes."""
import argparse,csv,hashlib,json,sys,tempfile
from dataclasses import dataclass
from pathlib import Path
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import ALL,B,ADJ,idx,flood,wave
from parallel_banks_anchored_cut_audit import bits,buf

H=10
@dataclass(frozen=True)
class Tube:
    k:int
    bank:int
    row:int
    l:int
    g:int
    z:int
    r:int
    inside:tuple
    wall:int
    @property
    def axis(self):return (self.l,self.g,*self.inside,self.z,self.r)
    @property
    def gates(self):return B[self.g]|B[self.z]
    @property
    def body(self):return buf(self.inside)

def parse_geometry(path):
    head,*lines=path.read_text().splitlines()
    rot,shift,_=map(int,head.split())
    tubes=[]
    for k,line in enumerate(lines):
        b,t,l,g,z,r,*rem=map(int,line.split())
        x=Tube(k,b,t,l,g,z,r,tuple(rem[:4]),buf(rem[4:]))
        assert len(rem)==12 and all(q in ADJ[p] for p,q in zip(x.axis,x.axis[1:]))
        tubes.append(x)
    return rot,shift,tubes

def nav(start,allowed,hmax=H):
    d={start:0};cur=B[start];seen=cur
    for s in range(1,hmax+1):
        cur=wave(cur)&allowed&~seen
        if not cur:break
        seen|=cur
        for p in bits(cur):d[p]=s
    return d

def recover(dst,d):
    assert dst in d
    path=[dst]
    while d[path[-1]]:
        q=path[-1]
        path.append(min(p for p in ADJ[q] if d.get(p)==d[q]-1))
    return path

def audit(name,rot,shift,tubes,history,cmds,meta):
    st=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in history]
    active={int(y.split()[0]) for y in meta.split(';') if y and int(y.split()[1]) in (1,2)}
    ready=[];windows=[];closures=[];current_empty=[]
    for t,(p,h,w) in enumerate(st):
        pets=buf(p);hbit=buf(h);blocked=pets|wave(pets)|hbit
        r=[x for x in tubes if x.wall&w==x.wall];ready.append(r)
        v={};clos=0;empty=[]
        for x in r:
            if (x.gates|x.body)&w:continue
            if not (x.gates|x.body)&(pets|hbit):empty.append(x)
            if x.gates&blocked or x.body&hbit:continue
            ids=[i for i,q in enumerate(p) if B[q]&x.body]
            if ids:
                assert all(p[i] in x.inside[1:3] for i in ids)
                v[x.k]=ids
                if t<300 and st[t+1][2]&x.gates==x.gates:clos|=x.gates
        windows.append(v);closures.append(clos);current_empty.append(empty)
    last=flood(buf(st[-1][1]),ALL^st[-1][2])
    missed={i for i,q in enumerate(st[-1][0]) if B[q]&last}
    resp=[];supply=[]
    # Measure only actual activated scheduling turns where current service
    # structure is visible; later observations are used only as a ceiling.
    for t in sorted(active):
        h=st[t][1];w=st[t][2]
        opens=ALL^(w|closures[t]);body=buf(set(q for x in ready[t] for q in x.inside))
        exterior=opens&~body
        ready_now={x.k for x in ready[t] if not (x.gates|x.body)&(w|closures[t])}
        if not current_empty[t] or not ready_now:continue
        live=[i for i in range(len(h)) if not (cmds[t][i].islower() and
              B[idx(step(divmod(h[i],30),STEPS[cmds[t][i].upper()]))]&closures[t])]
        ws={}
        for i in live:
            p=h[i]
            inside=next((x.body for x in ready[t] if x.body&B[p]),0)
            ws[i]=nav(p,exterior|(inside&opens))
        lane=lambda p:(p//30 if rot else p%30)
        nominal_pop={lane(x.l):sum(lane(p)==lane(x.l) for p in h) for x in tubes}
        nominal_pop.update({lane(x.r):sum(lane(p)==lane(x.r) for p in h) for x in tubes})
        bridges={}
        lower=max(active)
        # Prefix includes the actual open axial route to the other service
        # stance; no speculative intermediate movement through target bodies.
        for b in current_empty[t]:
            if b.k not in ready_now:continue
            for axial in (b.axis,b.axis[::-1]):
                fr,to=axial[0],axial[-1]
                for i in live:
                    if fr not in ws[i] or ws[i][fr]+7>H:continue
                    pp=recover(fr,ws[i])[::-1]+list(axial[1:])
                    arrive=len(pp)-1
                    if t+arrive>lower or any(j not in active for j in range(t,t+arrive+1)):continue
                    if any(B[q]&st[t+s][2] for s,q in enumerate(pp)):continue
                    if any(closures[j]&b.gates for j in range(t,t+arrive)):continue
                    bridges.setdefault(i,[]).append((b.k,to,pp))
                    if nominal_pop[lane(to)] or ws[i].get(to,99)<=arrive:continue
                    # Stable service capacity: number of already open actual
                    # other guard rows in the formerly empty lane within 3.
                    work=[x.k for x in ready[t] if x.k in ready_now and x.k!=b.k and
                          any(lane(a)==lane(to) and a!=to and abs(a//30-to//30)+abs(a%30-to%30)<=3
                              for a in (x.l,x.r))]
                    if work:
                        future=[(u,k) for u in range(t+arrive,min(t+arrive+16,lower+1))
                                for k in work if k in windows[u] and u in active]
                        supply.append(dict(turn=t,worker=i,bridge=b.k,lane=lane(to),eta=arrive,
                                           static_rows=len(set(work)),nearby_windows=len(future)))
        if not bridges:continue
        candidate={}
        nd={}
        for dt in range(7,H+1):
            u=t+dt
            if any(j not in active for j in range(t,u+1)):continue
            for k,targets in windows[u].items():
                if k not in ready_now:continue
                x=tubes[k]
                old_eta=min((max(ws[i].get(x.l,99),ws[j].get(x.r,99)) for i in live for j in live if i!=j),default=99)
                if old_eta<=dt:continue
                for left,right in [(x.l,x.r),(x.r,x.l)]:
                    if left not in nd:nd[left]=nav(left,exterior)
                    d=nd[left]
                    for i,opts in bridges.items():
                        for b,to,prefix in opts:
                            if b==k or to not in d:continue
                            pt=prefix+recover(to,d)[1:]
                            if len(pt)-1>dt:continue
                            if any(B[q]&st[t+s][2] for s,q in enumerate(pt)):continue
                            if any(B[pt[-1]]&st[t+s][2] for s in range(len(pt),dt+1)):continue
                            for j in live:
                                if i==j or ws[j].get(right,99)>dt:continue
                                jt=recover(right,ws[j])[::-1]
                                if any(B[q]&st[t+s][2] for s,q in enumerate(jt)):continue
                                if any(B[jt[-1]]&st[t+s][2] for s in range(len(jt),dt+1)):continue
                                missed_walls=late_locks=0
                                for tm in range(t,u+1):
                                    for actor in (i,j):
                                        a=cmds[tm][actor]
                                        if not a.islower():continue
                                        pos=st[tm][1][actor]
                                        q=idx(step(divmod(pos,30),STEPS[a.upper()]))
                                        missed_walls+=1
                                        late_locks+=bool(B[q]&closures[tm])
                                gain=[i for i in targets if i in missed]
                                z=dict(case=name,dispatch=t,finish=u,tube=k,bridge=b,crew=[i,j],
                                       old_eta=old_eta,new_eta=max(len(pt),len(jt))-1,targets=targets,
                                       final_missed=gain,replaced_walls=missed_walls,replaced_gate_jobs=late_locks,
                                       path=[pt,jt])
                                zz=(missed_walls,late_locks,z['new_eta'])
                                key=(u,k)
                                if key not in candidate or zz<candidate[key][0]:candidate[key]=(zz,z)
        resp.extend(z for _,z in candidate.values())
    representative={}
    for x in supply:
        x['case']=name
        k=(x['turn'],x['lane'])
        key=(x['nearby_windows'],x['static_rows'],-x['eta'])
        if k not in representative or key>representative[k][0]:representative[k]=(key,x)
    return resp,[x for _,x in representative.values()]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','tester','solver','cases','reference','opportunities','supply','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with open(a.reference) as f:refs=[r for r in csv.DictReader(f) if r['model']=='article']
    rr=[];ss=[]
    for r in refs:
        name=r['case'];c=(a.cases/name).read_bytes()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);geo=d/'geo';act=d/'act';trace=d/'trace';test=a.tester.resolve()
            o,e,rc,_=run([test,'env',f'AHC008_BANK_LOG={geo}',a.solver.resolve()],c,3.)
            assert rc==0 and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,trace],c,10.)
            assert (po,pe,pr)==(o,e,0)
            lines=trace.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(c.decode(),lines,cmds,detailed=True)[0]==int(r['score'])
            _,hist,_=observed(c,lines);rot,shift,tubes=parse_geometry(geo)
            x,y=audit(name,rot,shift,tubes,hist,cmds,r['telemetry']);rr+=x;ss+=y
        print(name,flush=True)
    distinct=lambda xs:len({(r['case'],x) for r in xs for x in r['final_missed']})
    strict=[r for r in rr if r['replaced_walls']==0]
    out=dict(verified=len(refs),supports_missing_lane=len(ss),
           support_states=len({(q['case'],q['turn'],q['lane']) for q in ss}),
           support_cases=len({q['case'] for q in ss}),
           sustainable_factual_opportunity_states=len({(q['case'],q['turn'],q['lane'])
                                                      for q in ss if q['nearby_windows']}),
           factual_witnesses=len(rr),factual_cases=len({r['case'] for r in rr}),
           factual_event_ids=len({(r['case'],r['finish'],r['tube']) for r in rr}),
           factual_distinct_final_missed=distinct(rr),strict_witnesses=len(strict),
           strict_distinct_final_missed=distinct(strict),
           strict_final_missed_cases=len({r['case'] for r in strict if r['final_missed']}),
           strict_gate_preserving_distinct=distinct([r for r in rr if not r['replaced_gate_jobs']]))
    for fp,data in [(a.opportunities,rr),(a.supply,ss)]:
        fp.write_text(''.join(json.dumps(x,separators=(',',':'))+'\n' for x in data))
    a.summary.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out,indent=2))
if __name__=='__main__':main()
