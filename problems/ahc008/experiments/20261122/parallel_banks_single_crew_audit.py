"""Sequential official paired dev benchmark; verify actions and isolate pet IDs."""
import argparse,csv,hashlib,json,sys,tempfile
from collections import Counter
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import B,ALL,idx,flood
from parallel_banks_anchored_cut_audit import buf,bits,rect

def captures(hist,species):
    prev=ALL
    new=[]
    for t,(pp,hh,w) in enumerate(hist):
        if t and w==hist[t-1][2]:continue
        assert len(hh)>0
        open_=ALL^buf(map(idx,w))
        root=flood(buf(map(idx,hh)),open_)
        for i,q in enumerate(map(idx,pp)):
            if B[q]&prev and not B[q]&root:
                pen=flood(B[q],open_)
                new.append(dict(turn=t-1,pet=i,type=species[i],area=pen.bit_count(),
                                nonrect=not rect(pen),cells=list(bits(pen))))
        prev=root
    return new

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','tester','cases','control','candidate','baseline_csv','output'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark import replay
    from benchmark_cases import run
    with a.baseline_csv.open() as f: old={r['case']:r for r in csv.DictReader(f) if r['model']=='narrow'}
    rows=[]
    with a.output.open('x') as f,tempfile.TemporaryDirectory() as td:
        d=Path(td)
        for ix,path in enumerate(sorted(a.cases.glob('*.txt'))):
            c=path.read_bytes()
            pairs={}
            for label,binary in ([('dual',a.control),('single',a.candidate)][::(-1 if ix%2 else 1)]):
                log=d/'log';log.unlink(missing_ok=True)
                tester=a.tester.resolve()
                o,e,rc,elapsed=run([tester,'env',f'AHC008_SINGLE_CREW_LOG={log}',binary.resolve()],c,3.)
                assert not rc and e.startswith(b'Score = '), (rc,e)
                score=int(e.split()[-1])
                eligible,release=map(int,log.read_text().split())
                act=d/'act';act.write_bytes(o)
                t=d/'trace'
                out2,e2,rc2,_=run([tester,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                                    '--playback',act,t],c,10.)
                assert (out2,e2,rc2)==(o,e,0)
                lines=t.read_text().splitlines()
                groups=[]
                s,humans,walls,largest=replay(c.decode(),lines,o.decode().splitlines(),topology=True,
                                             final_components=groups.extend)
                assert s==score
                species,hist,_=observed(c,lines)
                stops=captures(hist,species)
                counts=Counter(x['area'] for x in stops)
                assert len(stops)+sum(k for h,_,k in groups if h)==len(species)
                pairs[label]=(o,dict(case=path.name,arm=label,score=s,seconds=elapsed,eligible=eligible,
                                    release=release,sha256=hashlib.sha256(o).hexdigest(),humans=humans,
                                    walls=walls,largest=largest,isolated=stops,final4=sum(k for h,a,k in groups if k and not h and a==4),
                                    isolated4=counts[4],residual=sum(k for h,_,k in groups if h),
                                    clean_union=sum(a for h,a,k in groups if h and not k),
                                    human_union=sum(a for h,a,k in groups if h)))
            base,b=pairs['dual'];cand,g=pairs['single']
            assert b['release']==g['release']
            assert base.splitlines()[:b['release']]==cand.splitlines()[:b['release']]
            assert not b['eligible'] and not g['eligible'] or (b['release']==160)
            if b['eligible']==0:assert base==cand
            assert b['score']>=int(old[path.name]['score'])
            f.write(json.dumps([b,g],separators=(',',':'))+'\n');f.flush()
            rows.append((b,g));print(path.name,b['score'],g['score'],b['eligible'],g['eligible'],flush=True)
    assert len(rows)==len(old)==30
    for i in range(2):
        rr=[r[i] for r in rows]
        print(rr[0]['arm'],{k:sum(x[k] for x in rr) for k in ('score','eligible','isolated4','final4','residual','clean_union','human_union')},
              'active',sum(x['eligible']>0 for x in rr),'clean',sum(x['residual']==0 for x in rr),'max',max(x['seconds'] for x in rr))
if __name__=='__main__':main()
