"""Identical snapshots, stance availability prefilter in cut generation, read only."""
import argparse,csv,hashlib,json,sys,tempfile
from collections import Counter
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ADJ,B,ALL,idx,flood,wave
from parallel_banks_anchored_cut_audit import OFFSETS,protect,prep_cut,vertex_cut,staffing,buf,bits,distances,rect

def examine(hist,pets,name,t,rot,lanes,old):
    p0,h0,w=hist[t]
    pp=list(map(idx,p0));hh=list(map(idx,h0))
    p=buf(pp);h=buf(hh)
    opened=ALL^buf(map(idx,w));root=flood(B[hh[0]],opened)
    if h&root!=h:return []
    a,unused=protect(lanes,rot,root,hh)
    if not a:return []
    legal=root&~(h|p|wave(p));near=distances(h,root)
    approach=buf(q for q,d in enumerate(near) if d>=0 and d<=6)
    # Geometry-independent necessary condition: an adjacent place where
    # some worker could stand in <=6, and outside the exact forced pet core.
    out=[]
    for i,q in enumerate(pp):
        if not root&B[q] or a&B[q]:continue
        prior=old[name,t,i]
        if prior.get('nonrect',0) and prior.get('crew_eta') is not None:
            out.append(dict(case=name,turn=t,pet=i,status='already_routed'))
            continue
        forced=B[q]|wave(B[q])
        candidate=legal&wave(approach&~forced)
        network=prep_cut(root,a,candidate,near)
        cut=vertex_cut(network,root,q)
        row=dict(case=name,turn=t,pet=i,type=pets[i],old_status=prior['status'],
                 normally_missed=prior['normally_unresolved'])
        if not cut or cut.bit_count()>7:
            out.append({**row,'status':'none_at_most7'});continue
        assert cut&candidate==cut
        keep=flood(a&-a,root&~cut)
        cut&=wave(keep)
        if prior.get('walls')==list(bits(cut)):
            out.append({**row,'status':'identical_mask'});continue
        pen=flood(B[q],root&~cut)
        assert flood(a&-a,root&~cut)==keep and a&keep==a and not pen&h
        if pen.bit_count()>48:
            out.append({**row,'status':'changed_but_too_large','area':pen.bit_count()});continue
        age,crew=staffing(keep,cut,hh)
        row.update(status='novel_compact',nonrect=int(not rect(pen)),area=pen.bit_count(),eta=age,
                   cut=list(bits(cut)),pen=list(bits(pen)),crew=crew)
        out.append(row)
    return out

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','solver','inputs','control','previous','out','summary'):p.add_argument(n,type=Path)
    a=p.parse_args();sys.path.insert(0,str(a.harness.resolve()))
    from benchmark import replay
    from benchmark_cases import run
    with a.control.open() as f:ref=[r for r in csv.DictReader(f) if r['model']=='narrow']
    import gzip
    with gzip.open(a.previous,'rt') as f:
        old={(r['case'],r['turn'],r['pet']):r for r in map(json.loads,f)
             if r['kind']=='pet' and r['mode']=='strict'}
    results=[]
    for r in ref:
        name=r['case'];case=(a.inputs/name).read_bytes()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);meta=d/'geo';act=d/'act';trace=d/'trace'
            test=a.tester.resolve()
            stdout,e,rc,_=run([test,'env',f'AHC008_BANK_LOG={meta}',a.solver.resolve()],case,3.)
            assert rc==0 and e.strip()==('Score = '+r['score']).encode()
            assert r['actions_sha256']==hashlib.sha256(stdout).hexdigest()
            act.write_bytes(stdout)
            pb,e2,rc2,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                             '--playback',act,trace],case,10.)
            assert (pb,e2,rc2)==(stdout,e,0)
            lines=trace.read_text().splitlines()
            assert replay(case.decode(),lines,stdout.decode().splitlines(),detailed=True)[0]==int(r['score'])
            species,hist,_=observed(case,lines)
            txt=meta.read_text().splitlines();rot,shift,_=map(int,txt[0].split())
            n=max(int(f.split()[0]) for f in txt[1:])+1
            lanes=[shift+7*i for i in range(n+1)]
            for off in OFFSETS:results+=examine(hist,species,name,int(r['release'])+off,rot,lanes,old)
        print(name,flush=True)
    nov=[r for r in results if r['status']=='novel_compact' and r['nonrect']]
    ok=[r for r in nov if r['eta'] is not None]
    miss={(r['case'],r['pet']) for r in ok if r['normally_missed']}
    s=dict(verified=len(ref),events=len(results),stages=dict(Counter(r['status'] for r in results)),
           novel_nonrect_events=len(nov),novel_nonrect_cases=len({r['case'] for r in nov}),
           novel_nonrect_routed=len(ok),novel_nonrect_routed_cases=len({r['case'] for r in ok}),
           novel_missed_routed=len(miss),novel_missed_cases=len({r[0] for r in miss}))
    a.out.write_text(''.join(json.dumps(r,separators=(',',':'))+'\n' for r in results))
    a.summary.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))
if __name__=='__main__':main()
