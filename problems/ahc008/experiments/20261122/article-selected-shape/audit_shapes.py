"""Unchanged-output verification and selected late-shape execution accounting."""
import argparse,csv,hashlib,json,collections,sys,tempfile
from pathlib import Path
P='turn attempt kind gates humans maxOpen maxOutside maxMatch assignedSum proxyCost remaining builds walks tier kept trap'.split()
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','solver','inputs','reference','out','summary'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with open(a.reference) as f: refs=[r for r in csv.DictReader(f) if r['model']=='article']
    ledger=[]
    for r in refs:
        name=r['case'];case=(a.inputs/name).read_bytes()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);x=d/'log';t=a.tester.resolve()
            o,e,rc,_=run([t,'env',f'AHC008_LAST_SHAPE_LOG={x}',a.solver.resolve()],case,3.)
            assert rc==0 and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act=d/'act';act.write_bytes(o)
            tr=d/'tr'
            o2,e2,rc2,_=run([t,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                              '--playback',act,tr],case,10.)
            assert (o2,e2,rc2)==(o,e,0)
            lines=tr.read_text().splitlines()
            assert replay(case.decode(),lines,o.decode().splitlines(),detailed=True)[0]==int(r['score'])
            event=collections.defaultdict(list)
            for q in r['captures'].split(';'):
                if q:
                    t,i,ty,ar,non=map(int,q.split(':'));event[t-1].append([i,ty,ar,non])
            steps=collections.defaultdict(list)
            for line in x.read_text().splitlines():
                q=dict(zip(P,map(int,line.split()),strict=True))
                steps[q['attempt']].append(q)
            count=0
            for v in steps.values():
                first=v[0]
                assert first['maxOpen']>=0 and first['maxOutside']>=first['maxOpen']
                if first['gates']<=first['humans']: assert first['maxMatch']>=first['maxOutside']
                own=[q for q in v if q['kind']>=0]
                count+=len(own)
                hits=[[q['turn'],*z] for q in own for z in event[q['turn']]]
                ledger.append(dict(case=name,final_miss=int(r['final_reachable']),selected=first,owned=own,
                                   actual=hits,atomic=sum(q['kind']==2 for q in own)))
            assert count==sum(int(x.split()[1]) in (3,5) for x in r['telemetry'].split(';') if x)
        print(name,flush=True)
    n=lambda c:sum(c(q) for q in ledger)
    small=[r for r in ledger if r['selected']['gates']<=r['selected']['humans']]
    vs=[r for r in small if r['selected']['maxMatch']+1>r['selected']['proxyCost']]
    vs5=[r for r in small if r['selected']['maxMatch']+1>=r['selected']['proxyCost']+5]
    s=dict(verified=len(refs),attempts=len(ledger),cases=len({r['case'] for r in ledger}),
           small=len(small),larger=len(ledger)-len(small),captured_attempts=n(lambda r:bool(r['actual'])),
           owned_turns=sum(len(r['owned']) for r in ledger),
           actual_captures=sum(len(r['actual']) for r in ledger),
           max_matching_slower_than_proxy=len(vs),max_matching_at_least_5_slower=len(vs5),
           max_matching_exceeds_remaining=sum(r['selected']['maxMatch']+1>r['selected']['remaining'] for r in small),
           slowed_cases=len({r['case'] for r in vs}),
           slowed_5_cases=len({r['case'] for r in vs5}),
           slowed_5_final_missed_cases=len({r['case'] for r in vs5 if r['final_miss']}))
    a.out.write_text(''.join(json.dumps(r,separators=(',',':'))+'\n' for r in ledger))
    a.summary.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))
if __name__=='__main__':main()
