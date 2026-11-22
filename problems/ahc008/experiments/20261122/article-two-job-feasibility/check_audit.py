"""Validate source-preserving first-state two-job schedules against official tapes."""
import argparse,csv,hashlib,json,sys,tempfile
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,flood,wave
from parallel_banks_anchored_cut_audit import buf,bits,distances,rect
INF=100000000

def audit(log,history,species,name):
    rows=[]
    span=lambda t:[list(map(idx,history[t][0])),list(map(idx,history[t][1])),
                   buf(map(idx,history[t][2]))]
    seen=set()
    for line in log.read_text().splitlines():
        typ,t,i,mask,*rest=map(int,line.split())
        assert 160<=t<205
        if typ==0:
            assert not seen
            seen.add((t,i,mask));rows.append(dict(case=name,status='considered',turn=t,pet=i,crew=mask.bit_count()))
            continue
        assert (t,i,mask) in seen
        k,time,blocked,*data=rest
        cut=data[:k];raw=data[k:];walls=buf(cut)
        pp,hh,ww=span(t);op=ALL^ww
        before=flood(B[hh[0]],op)
        after=flood(B[hh[0]],op&~walls)
        assert buf(hh)&after==buf(hh)
        pen=flood(B[pp[i]],op&~walls)
        assert 4<=pen.bit_count()<=48 and not rect(pen) and not pen&after
        for p in pp:
            if B[p]&before and not B[p]&after:
                assert flood(B[p],op&~walls).bit_count()<=48
        hcount=mask.bit_count()
        assert len(raw)==8*hcount and k<=2*hcount
        assert (walls&(buf(pp)|wave(buf(pp))|buf(hh))).bit_count()==blocked
        q=dict(case=name,status='geometry',turn=t,pet=i,type=species[i],crew=hcount,
               walls=cut,area=pen.bit_count(),cells=list(bits(pen)),time=time,static_blocked=blocked)
        if time>=INF:
            rows.append(q)
            continue
        jobs=0;plans=[]
        comp=0
        for x in range(0,len(raw),8):
            human,a,b,sa,sb,eta,d1,d2=raw[x:x+8];assert mask>>human&1
            if a<0:assert b<0;continue
            assert B[sa]&after and B[sa]&wave(B[cut[a]])
            assert not jobs>>a&1;jobs|=1<<a
            assert distances(B[hh[human]],after)[sa]==d1
            if b>=0:
                assert not jobs>>b&1;jobs|=1<<b
                assert B[sb]&after and B[sb]&wave(B[cut[b]])
                assert distances(B[sa],after)[sb]==d2 and eta==d1+d2+2
                plans.extend([(d1,a,human,sa),(time-1,b,human,sb)])
            else:
                assert eta==d1+1
                plans.append((time-1,a,human,sa))
            comp=max(comp,eta)
        assert jobs==(1<<k)-1 and comp==time
        for g in range(k):
            assert sum(job[1]==g for job in plans)==1
        due=(time<=205-t)
        q.update(within=due,plan=sorted(plans))
        if due:
            end=t+time-1
            q['factual_inside_at_close']=bool(B[span(end)[0][i]]&pen)
            q['factual_never_exited']=all(B[span(l)[0][i]]&pen for l in range(t,end+1))
            q['factual_blocked_installs']=sum(bool(B[cut[j]]&(buf(span(t+s)[0])|wave(buf(span(t+s)[0]))))
                                              for s,j,_,_ in plans)
        rows.append(q)
    return rows

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','tester','solver','cases','reference','out','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with open(a.reference) as f:refs=[r for r in csv.DictReader(f) if r['model']=='article']
    rows=[]
    for r in refs:
        name=r['case'];case=(a.cases/name).read_bytes()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);lt=d/'log';tr=d/'tape';act=d/'act';tester=a.tester.resolve()
            o,e,rc,_=run([tester,'env',f'AHC008_TWO_ROUTE_LOG={lt}',a.solver.resolve()],case,3.)
            assert rc==0 and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([tester,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tr],case,10.)
            assert (po,pe,pr)==(o,e,0)
            tape=tr.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(case.decode(),tape,cmds,detailed=True)[0]==int(r['score'])
            species,history,_=observed(case,tape)
            rows+=audit(lt,history,species,name)
        print(name,flush=True)
    states=[r for r in rows if r['status']=='considered']
    geo=[r for r in rows if r['status']=='geometry']
    short=[r for r in geo if r.get('within')]
    bonus=[r for r in short if len(r['walls'])>r['crew']]
    count=lambda xs:len({r['case'] for r in xs})
    s=dict(verified=len(refs),screened=len(states),geometries=len(geo),geo_cases=count(geo),
           ready_by_deadline=len(short),cases_by_deadline=count(short),new_gt_crew=len(bonus),
           new_cases=count(bonus),static_unblocked_new=sum(not r['static_blocked'] for r in bonus),
           observed_resident_new=sum(r['factual_inside_at_close'] for r in bonus),
           observed_always_resident_new=sum(r['factual_never_exited'] for r in bonus),
           observed_resident_and_legal_new=sum(r['factual_inside_at_close'] and not r['factual_blocked_installs'] for r in bonus))
    a.out.write_text(''.join(json.dumps(r,separators=(',',':'))+'\n' for r in rows))
    a.summary.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))
if __name__=='__main__':main()
