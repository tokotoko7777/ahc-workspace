"""Unchanged-output fallback evacuation ETA and actual frontier audit."""
import argparse,csv,hashlib,json,sys,tempfile
from collections import defaultdict
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import B,ALL,idx,flood,wave

def from_kept(kept, open_, humans):
    targets={h for h in humans}
    distances={}
    seen=front=kept&open_
    d=0
    while front and targets:
        for p in list(targets):
            if front&B[p]:distances[p]=d;targets.remove(p)
        front=wave(front)&open_&~seen
        seen|=front
        d+=1
    return distances

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','instrumented','cases','control','output','json'):
        p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with a.control.open() as f:
        ref=[r for r in csv.DictReader(f) if r['model']=='narrow']
    out=[]
    for r in ref:
        path=a.cases/r['case']; case=path.read_bytes()
        with tempfile.TemporaryDirectory() as d:
            b=Path(d)/'evac'
            cmd=[a.tester.resolve(),'env',f'AHC008_EVAC_AUDIT={b}',a.instrumented.resolve()]
            acts,err,rc,_=run(cmd,case,3.)
            assert rc==0 and err.strip()==('Score = '+r['score']).encode(),(r['case'],err)
            assert hashlib.sha256(acts).hexdigest()==r['actions_sha256'],r['case']
            af=Path(d)/'actions';trace=Path(d)/'trace';af.write_bytes(acts)
            play,err2,rc2,_=run([a.tester.resolve(),sys.executable,a.harness.resolve()/'benchmark_cases.py',
                                '--playback',af,trace],case,10.)
            assert (play,err2,rc2)==(acts,err,0)
            lines=trace.read_text().splitlines()
            assert int(r['score'])==replay(case.decode(),lines,acts.decode().splitlines(),detailed=True)[0]
            _,hist,_=observed(case,lines)
            grid=[ALL^sum(B[idx(p)] for p in x[2]) for x in hist]
            rows=[]
            for line in b.read_text().splitlines() if b.exists() else []:
                values=list(map(int,line.split()))
                names=('turn','line_id','plan_start','builder_eta','jobs','queued','mobile','kept_people',
                       'other_groups','captive_decoy','evac_eta','proposed_extra_delay','proposed_blocked',
                       'wall_jobs_accessible_from_kept','available_people_in_kept','kept_cell')
                row=dict(zip(names,values))
                t=row['turn']
                assert t>=int(r['release']) and row['captive_decoy']==0
                planned=sum(B[p] for p in values[len(names):])
                assert planned.bit_count()==row['jobs'] and planned&grid[t]==planned
                kept=flood(B[row['kept_cell']],grid[t]&~planned)
                room=flood(B[row['kept_cell']],grid[t])
                hh=[idx(p) for p in hist[t][1] if room&B[idx(p)]]
                outside=[p for p in hh if not (kept&B[p])]
                old=from_kept(kept,grid[t],outside)
                new=from_kept(kept,grid[t+1],outside)
                assert len(old)==len(set(outside))
                assert max(old.values(),default=0)==row['evac_eta']
                assert len(hh)==row['mobile'] and len(hh)-len(outside)==row['kept_people']
                row.update(case=r['case'],new_walls_verified=(planned&(grid[t]^grid[t+1])).bit_count(),
                           remaining_after=(planned&grid[t+1]).bit_count(),actual_blocked=sum(p not in new for p in outside),
                           actual_extra_delay=max((new[p]-old[p] for p in outside if p in new),default=0))
                rows.append(row)
            seq=defaultdict(list)
            for row in rows:seq[row['plan_start']].append(row)
            for episode,arr in seq.items():
                completed=any(x['remaining_after']==0 for x in arr)
                for j,row in enumerate(arr):
                    row['episode_completed']=int(completed)
                    row['episode_last_turn']=arr[-1]['turn']
                    row['commit']=int(j==0 and row['turn']==row['plan_start'] and row['builder_eta']>=0)
                out.extend(arr)
    with a.output.open('w') as f:
        w=csv.DictWriter(f,list(out[0]),lineterminator='\n');w.writeheader();w.writerows(out)
    commit=[r for r in out if r['commit']]
    over=[r for r in commit if r['evac_eta']>=r['builder_eta']+5 or r['turn']+r['evac_eta']>=300]
    def num(r): return len(r),len({x['case'] for x in r})
    summary=dict(verified_cases=len(ref),selected_ticks=len(out),commit_calls=num(commit),
        evac_required_calls=num([r for r in commit if r['evac_eta']>0]),
        significant_eta_calls=num(over),
        significant_completed=num([r for r in over if r['episode_completed']]),
        significant_not_completed=num([r for r in over if not r['episode_completed']]),
        deadline_fail=num([r for r in over if r['turn']+r['evac_eta']>=300]),
        actual_route_removed=num([r for r in out if r['actual_blocked']]),
        actual_route_delayed=num([r for r in out if r['actual_extra_delay']>0]),
        proposed_route_removed=num([r for r in out if r['proposed_blocked']]),
        no_current_builder_at_commit=num([r for r in commit if r['available_people_in_kept']==0]),
        no_kept_side_stance_for_all_jobs=num([r for r in commit if r['wall_jobs_accessible_from_kept']<r['jobs']]))
    a.json.write_text(json.dumps(summary,indent=2)+'\n');print(json.dumps(summary,indent=2))
if __name__=='__main__':main()
