"""No counterfactual rollout: audit immediately legal old bank gates after rescue handoff."""
import argparse, csv, hashlib, itertools, json, sys, tempfile
from collections import Counter
from pathlib import Path
from guarded_entry_trace import observed, step, STEPS
from stochastic_lock_audit import ALL, B, idx, wave, flood

def audit(case, species, hist, commands, geometry, release):
    layout=[]
    for line in geometry:
        _,_,left,gate1,gate2,right,*cells=map(int,line.split())
        inner=sum(B[q] for q in cells[:4])
        brace=sum(B[q] for q in cells[4:])
        assert wave(inner)&~inner == brace | B[gate1] | B[gate2]
        layout.append((left,gate1,gate2,right,inner,brace))
    petpos,humanpos,wall=hist[-1]
    final=flood(sum({B[idx(h)] for h in humanpos}),ALL^sum(B[idx(q)] for q in wall))
    unresolved={i for i,p in enumerate(petpos) if final&B[idx(p)]}
    for t in range(release,300):
        pets,people,walls=hist[t]
        _,_,next_walls=hist[t+1]
        occ=sum({B[idx(p)] for p in pets})
        human=sum({B[idx(p)] for p in people})
        forbid=wave(occ)|occ|human
        blocked=sum(B[idx(q)] for q in walls)
        ordinary=sum(B[idx(q)] for q in next_walls)
        builds=ordinary^blocked
        assert not builds&blocked
        before=flood(human,ALL^blocked)
        by_pos={}
        for i,p in enumerate(people):by_pos.setdefault(idx(p),[]).append(i)
        act=commands[t]
        nextp=[idx(step(q,STEPS[c])) if c in STEPS else idx(q) for q,c in zip(people,act)]
        normal=flood(sum({B[p] for p in nextp}),ALL^ordinary)
        for k,(left,g1,g2,right,inner,brace) in enumerate(layout):
            gates=B[g1]|B[g2]
            if not inner&occ&before or brace&blocked!=brace or gates&(blocked|forbid) or inner&(human|blocked):
                continue
            target=[i for i,p in enumerate(pets) if inner&B[idx(p)]]
            a,b=by_pos.get(left,[]),by_pos.get(right,[])
            available=bool(a and b)
            already_isolated=not (normal&inner)
            keep_wall=[];no_strand=[];no_move=[]
            base_captures=builds&gates==gates
            if available:
                for i,j in itertools.product(a,b):
                    assert i!=j
                    if any(act[w].islower() and idx(step(people[w],STEPS[act[w].upper()])) not in (g1,g2)
                        for w in (i,j)):
                        continue
                    keep_wall.append((i,j))
                    after=ordinary|gates
                    post=nextp.copy();post[i]=left;post[j]=right
                    if inner&after or any(B[p]&after for p in post):continue
                    reachable=flood(B[post[0]],ALL^after)
                    if any(not reachable&B[p] for p in post):continue
                    assert not reachable&inner
                    assert flood(inner&-inner,ALL^after)==inner
                    no_strand.append((i,j))
                    if not any(act[w] in STEPS for w in (i,j)):
                        no_move.append((i,j))
            for i in target:
                yield dict(case=case,turn=t,tube=k,pet=i,species=species[i],
                           staffed=int(available),wall_preserved=int(bool(keep_wall)),
                           connected=int(bool(no_strand)),no_move_lost=int(bool(no_move)),
                           ordinary_already_both=int(base_captures),ordinary_already_isolated=int(already_isolated),
                           final_unresolved=int(i in unresolved),
                           builders=';'.join(f'{x} {y}' for x,y in no_strand))


def summary(rows):
    out={}
    for k,ind in [('legal',lambda r:True),('staffed',lambda r:r['staffed']),
                  ('preserve_walls',lambda r:r['wall_preserved']),('connected',lambda r:r['connected']),
                  ('no_move_lost',lambda r:r['no_move_lost'])]:
        use=[r for r in rows if ind(r)]
        missed={(r['case'],r['pet']):r['species'] for r in use if r['final_unresolved']}
        caught={(r['case'],r['pet']) for r in use if not r['final_unresolved']}
        out[k]=dict(tube_turns=len({(r['case'],r['turn'],r['tube']) for r in use}),
                    cases=len({r['case'] for r in use}), unique_missed=len(missed),
                    cases_missed=len({c for c,_ in missed}),
                    missed_by_species=dict(Counter(missed.values())),
                    unique_normally_caught=len(caught),
                    already_normally_isolated_events=sum(r['ordinary_already_isolated'] for r in use),
                    already_atomic_events=sum(r['ordinary_already_both'] for r in use))
    return out


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('harness','tester','solver','cases','control_csv','output','json'):
        p.add_argument(name,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with a.control_csv.open() as f:
        ref={r['case']:r for r in csv.DictReader(f) if r['model']=='rescue160'}
    rows=[]
    casefiles=sorted(a.cases.glob('*.txt'))[:30]
    assert len(casefiles)==30
    for f in casefiles:
        with tempfile.TemporaryDirectory() as td:
            q=Path(td)/'meta'; releasepath=Path(td)/'rescue'
            cmd=[a.tester.resolve(),'env',f'AHC008_BANK_LOG={q}',f'AHC008_RESCUE_LOG={releasepath}',a.solver.resolve()]
            actions,err,rc,_=run(cmd,f.read_bytes(),3.)
            prior=ref[f.name]
            assert rc==0 and err.strip()==('Score = '+prior['score']).encode()
            assert hashlib.sha256(actions).hexdigest()==prior['actions_sha256']
            blob=Path(td)/'actions';trace=Path(td)/'trace';blob.write_bytes(actions)
            play,err2,rc2,_=run([a.tester.resolve(),sys.executable,a.harness.resolve()/'benchmark_cases.py',
                                 '--playback',blob,trace],f.read_bytes(),10.)
            assert (play,err2,rc2)==(actions,err,0)
            commands=actions.decode().splitlines(); logs=trace.read_text().splitlines()
            score,*_=replay(f.read_text(),logs,commands,detailed=True)
            assert score==int(prior['score'])
            kinds,hist,_=observed(f.read_bytes(),logs)
            _,*geometry=q.read_text().splitlines()
            n160,release=map(int,releasepath.read_text().split())
            assert release==(160 if n160>4 else 225)
            rows.extend(audit(f.name,kinds,hist,commands,geometry,release))
        print(f.name,flush=True)
    with a.output.open('w') as f:
        wr=csv.DictWriter(f,list(rows[0]),lineterminator='\n');wr.writeheader();wr.writerows(rows)
    a.json.write_text(json.dumps(summary(rows),indent=2)+'\n')

if __name__=='__main__':main()
