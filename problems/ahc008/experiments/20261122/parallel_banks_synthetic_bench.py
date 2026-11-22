"""Offline-only paired hypothetical continuations versus exact used-dev realized branches."""
import argparse,csv,hashlib,json,statistics,sys,tempfile
from pathlib import Path

SEEDS=[0xb16b00b5,0x08c0ffee,0xd159d00f,0x49a008ff]

def verify(run,replay,tester,harness,cmd,case,output,scorecheck,timeout):
    act,err,rc,wall=run(cmd,case,timeout)
    assert rc==0 and err.startswith(b'Score = '),(rc,err)
    score=int(err.split()[2])
    assert score==int(scorecheck.read_text())
    a=output/'actions';tr=output/'trace';a.write_bytes(act)
    pb,err2,rc2,_=run([tester,sys.executable,harness/'benchmark_cases.py','--playback',a,tr],case,10.)
    assert (act,err,rc)==(pb,err2,rc2)
    assert score==replay(case.decode(),tr.read_text().splitlines(),act.decode().splitlines(),detailed=True)[0]
    return score,act,wall

def summarize(rows,shadow):
    active=[r for r in rows if r['active']]
    totals=dict(cases=len(rows),active=len(active),
                current=sum(r['current'] for r in rows),always_stay=sum(r['stay'] for r in rows),
                chosen=sum(r['score_picked'] for r in rows),oracle=sum(r['oracle'] for r in rows),
                stay_picks=sum(r['pick_stay'] for r in rows),
                wins=sum(r['delta']>0 for r in rows),losses=sum(r['delta']<0 for r in rows),
                gross_gain=sum(max(0,r['delta']) for r in rows),gross_loss=sum(max(0,-r['delta']) for r in rows),
                current_regret=sum(r['oracle']-r['current'] for r in rows),
                chosen_regret=sum(r['oracle']-r['score_picked'] for r in rows),
                half_pick_diff=sum(r['half01_stay']!=r['half23_stay'] for r in active),
                first_half_actual_total=sum(r['stay'] if r['half01_stay'] else r['current'] for r in rows),
                second_half_actual_total=sum(r['stay'] if r['half23_stay'] else r['current'] for r in rows),
                active_wall_median=statistics.median([r['wall_observed'] for r in active] or [0]),
                active_wall_max=max([r['wall_observed'] for r in active] or [0]),
                pure_shadow_cpu_median=statistics.median([r['sum_cpu'] for r in active] or [0]),
                pure_shadow_cpu_max=max([r['sum_cpu'] for r in active] or [0]),
                max_one_roll_cpu=max([max(r['cpu_early'],r['cpu_stay']) for r in shadow] or [0]),
                total_check_polls=sum(r['polls_early']+r['polls_stay'] for r in shadow),
                max_one_roll_polls=max([max(r['polls_early'],r['polls_stay']) for r in shadow] or [0]),
                total_clock_cutoffs=sum(r['trips_early']+r['trips_stay'] for r in shadow),
                max_one_roll_cutoffs=max([max(r['trips_early'],r['trips_stay']) for r in shadow] or [0]))
    return totals

def write_csv(path,rows):
    with path.open('w') as f:
        w=csv.DictWriter(f,list(rows[0]),lineterminator='\n');w.writeheader();w.writerows(rows)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','binary','cases','control','out_cases','out_rolls','out_summary'):
        p.add_argument(n,type=Path)
    p.add_argument('--limit',type=int,default=30)
    a=p.parse_args()
    a.harness=a.harness.resolve();a.binary=a.binary.resolve();a.tester=a.tester.resolve()
    sys.path.insert(0,str(a.harness))
    from benchmark_cases import run
    from benchmark import replay
    with a.control.open() as f:
        ref=[r for r in csv.DictReader(f) if r['model']=='narrow'][:a.limit]
    rows=[];shadow=[]
    for r in ref:
        name=r['case'];case=(a.cases/name).read_bytes()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);diag=d/'predict';sc=d/'score'
            command=[a.tester,'env',f'AHC008_SYNTHETIC_LOG={diag}',f'AHC008_VALIDATE_MOVES={sc}',a.binary]
            official,actions,wall=verify(run,replay,a.tester,a.harness,command,case,d,sc,40.)
            assert official==int(r['score']) and hashlib.sha256(actions).hexdigest()==r['actions_sha256']
            data=diag.read_text().splitlines()
            n,reps=map(int,data[0].split())
            assert n==int(r['residual160']) and reps==len(SEEDS)
            active=n>4
            assert len(data)==(1+reps if active else 1)
            sim=[]
            for rr,line in enumerate(data[1:]):
                spl=line.split(); seed,e,s=map(int,spl[:3]);u,v=map(float,spl[3:5])
                ex,st,d1,d2,c1,c2,t1,t2,p1,p2,b1,b2=map(int,spl[5:])
                assert seed==SEEDS[rr]
                sim.append(dict(case=name,rep=rr,seed=seed,early=e,stay=s,paired_delta=s-e,cpu_early=u,cpu_stay=v,
                       turns_over_early=ex,turns_over_stay=st,dog_fallback_early=d1,dog_fallback_stay=d2,
                       cat_retargets_early=c1,cat_retargets_stay=c2,dog_retargets_early=t1,dog_retargets_stay=t2,
                       polls_early=p1,polls_stay=p2,trips_early=b1,trips_stay=b2))
            shadow.extend(sim)
            if active:
                command=[a.tester,'env',f'AHC008_VALIDATE_MOVES={sc}',a.binary,'5','0','1']
                stay,sa,sw=verify(run,replay,a.tester,a.harness,command,case,d,sc,10.)
                assert actions.splitlines()[:160]==sa.splitlines()[:160]
                stay_hash=hashlib.sha256(sa).hexdigest()
            else:
                stay=official;stay_hash=r['actions_sha256'];sw=0.
            dlt=[x['paired_delta'] for x in sim];choice=sum(dlt)>0
            st_score=stay if choice else official
            rows.append(dict(case=name,active=int(active),residual160=n,current=official,stay=stay,
                 oracle=max(official,stay),score_picked=st_score,delta=st_score-official,pick_stay=int(choice),
                 mean_simulated_delta=statistics.mean(dlt) if dlt else 0,
                 simulated_deltas=';'.join(map(str,dlt)),half01_stay=int(sum(dlt[:2])>0),half23_stay=int(sum(dlt[2:])>0),
                 sum_cpu=sum(x['cpu_early']+x['cpu_stay'] for x in sim),wall_observed=wall,wall_stay=sw,
                 actions_sha256=r['actions_sha256'],stay_actions_sha256=stay_hash))
        print(name,flush=True)
        write_csv(a.out_cases,rows)
        if shadow:write_csv(a.out_rolls,shadow)
    totals=summarize(rows,shadow)
    a.out_summary.write_text(json.dumps(totals,indent=2)+'\n')
    print(json.dumps(totals,indent=2))
if __name__=='__main__':main()
