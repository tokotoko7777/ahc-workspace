"""Matched small initial lastcap shapes: exact passive endpoint retention audit."""
import argparse,csv,hashlib,json,sys,tempfile,collections,importlib.util
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,ADJ,idx,flood
from parallel_banks_anchored_cut_audit import buf,bits
from check import component

def retain(terrain,origin,pen,substeps,horizon):
    q={origin:1.}
    edge={}
    for _ in range(horizon):
        for step in range(substeps):
            final=collections.defaultdict(float)
            for c,mass in q.items():
                if c not in edge:
                    edge[c]=tuple(p for p in ADJ[c] if B[p]&terrain) or (c,)
                nn=edge[c];a=mass/len(nn)
                for p in nn:final[p]+=a
            q=final
        q={p:f for p,f in q.items() if B[p]&pen}
    return sum(q.values())

def inspect(name,log,hist,species,match):
    s=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in hist]
    groups=collections.defaultdict(set)
    start=[]
    for z in log.read_text().splitlines():
        kind,t,key,tier,*c=map(int,z.split())
        if kind==1:groups[t,tier].add(buf(c))
        if kind==2:start.append((t,tier,buf(c)))
    count=collections.Counter()
    rows=[]
    for t,tier,chosen in start:
        count['selected']+=1
        pp,hh,w=s[t];op=ALL^w;live=flood(B[hh[0]],op)
        for i,p in enumerate(pp):
            if species[i]>3 or not B[p]&live:continue
            base=component(chosen,w,p,hh)
            if not base:continue
            count['selected_small_passive']+=1
            ring,body=base
            jobs=list(bits(chosen&~w))
            packed=match(hh,jobs,ring)
            if packed is None:continue
            d,_=packed
            if not 1<=d<=6 or t+d>299:continue
            count['selected_small_passive_short']+=1
            choices=groups[t,tier]
            ba=body.bit_count()
            for alt in choices:
                if alt==chosen or len(jobs)!=(alt&~w).bit_count():continue
                shape=component(alt,w,p,hh)
                if not shape:continue
                otherRing,otherBody=shape
                ar=otherBody.bit_count()
                if abs(ba-ar)>max(1,min(ba,ar)//5):continue
                count['matched_geometry_not_yet_eta']+=1
                route=match(hh,list(bits(alt&~w)),otherRing)
                if route is None or route[0]!=d:continue
                count['matched']+=1
                expected=[retain(op,p,z,species[i],d) for z in [body,otherBody]]
                actual=[all(B[s[j][0][i]]&z for j in range(t+1,t+d+1)) for z in [body,otherBody]]
                rows.append(dict(case=name,turn=t,pet=i,type=species[i],walls=len(jobs),eta=d,
                          area=[ba,ar],retained=[ring.bit_count(),otherRing.bit_count()],
                          probability=expected,factual_retained=actual,
                          original_cut=list(bits(chosen)),alt_cut=list(bits(alt))))
    return rows,count

def selfcheck():
    # Board line 0-1-2. Pig may leave temporary {0} at substep 1,
    # come back with half mass by substep 2; observe only endpoint.
    assert retain(buf((0,1,2)),0,B[0],2,1)==.5
    assert retain(buf((0,1,2)),0,B[0],2,2)==.25
    assert retain(buf((0,1,2)),0,B[0],1,1)==0.

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','wide_audit','tester','solver','cases','reference','out','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    spec=importlib.util.spec_from_file_location('wide_audit',a.wide_audit.resolve())
    wide=importlib.util.module_from_spec(spec);spec.loader.exec_module(wide)
    selfcheck()
    with open(a.reference) as f:refs=[r for r in csv.DictReader(f) if r['model']=='article']
    out=[];counts=collections.Counter()
    for r in refs:
        tag=r['case'];data=(a.cases/tag).read_bytes();tt=a.tester.resolve()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);lt=d/'log';tr=d/'tape';act=d/'act'
            o,e,rc,_=run([tt,'env',f'AHC008_RETRY_AUDIT={lt}',a.solver.resolve()],data,3.)
            assert not rc and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([tt,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tr],data,10.)
            assert (po,pe,pr)==(o,e,0)
            lines=tr.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(data.decode(),lines,cmds,detailed=True)[0]==int(r['score'])
            species,hist,_=observed(data,lines)
            z,c=inspect(tag,lt,hist,species,wide.match);out+=z;counts.update(c)
        print(tag,flush=True)
    comp=[r for r in out if r['factual_retained'][0]!=r['factual_retained'][1]]
    outsum=dict(verified=len(refs),counts=counts,matched_cases=len({r['case'] for r in out}),
            matched_exact_area=sum(r['area'][0]==r['area'][1] for r in out),
            factual_informative_pairs=len(comp),factual_informative_cases=len({r['case'] for r in comp}))
    a.out.write_text(''.join(json.dumps(r,separators=(',',':'))+'\n' for r in out))
    a.summary.write_text(json.dumps(outsum,indent=2)+'\n');print(json.dumps(outsum,indent=2))
if __name__=='__main__':main()
