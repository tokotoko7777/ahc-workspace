"""Verify strict idle ready-bridge eligibility without changing v4 actions."""
import argparse,csv,hashlib,json,sys,tempfile
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import B,idx
from parallel_banks_anchored_cut_audit import buf

def examine(name,log,geo,hist,cmds):
    from bridge import parse_geometry
    rot,_,tubes=parse_geometry(geo)
    st=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in hist]
    lane=lambda q:q//30 if rot else q%30
    rows=[]
    for line in log.read_text().splitlines():
        t,who,i,start,end,supply,full=map(int,line.split())
        p,h,w=st[t];x=tubes[i]
        assert 101<=t<=152 and h[who]==start
        close=x.gates | x.body
        assert close&(w|buf(p)|buf(h))==0
        assert x.wall&w==x.wall and sum(lane(q)==lane(start) for q in h)>=2
        assert all(lane(q)!=lane(end) for q in h)
        assert full==all(y.wall&w==y.wall for y in tubes)
        axial=x.axis if x.axis[0]==start else x.axis[::-1]
        assert len(axial)==8 and axial[-1]==end
        route_open=all(not B[q]&st[t+j][2] for j,q in enumerate(axial))
        no_reserved_gate=not x.gates&st[t+7][2]
        donor_stays=all(any(j!=who and lane(q)==lane(start) for j,q in enumerate(st[u][1])) for u in range(t,t+8))
        parked=cmds[t][who]!='.'
        replaced=sum(cmds[u][who].islower() for u in range(t,t+7))
        rows.append(dict(case=name,turn=t,worker=who,bridge=i,source=lane(start),dest=lane(end),
                         support=supply,all_ready=full,actual_parking=parked,route_open=route_open,
                         reserve_nonconflict=no_reserved_gate,donor_stays=donor_stays,ordinary_builds=replaced))
    return rows

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','bridge_audit','tester','solver','cases','reference','out','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path[:0]=[str(a.harness.resolve()),str(a.bridge_audit.resolve())]
    from benchmark_cases import run
    from benchmark import replay
    with open(a.reference) as f:refs=[r for r in csv.DictReader(f) if r['model']=='article']
    rows=[]
    for rr in refs:
        name=rr['case'];data=(a.cases/name).read_bytes()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);log=d/'log';geo=d/'geo';act=d/'actions';trace=d/'trace';t=a.tester.resolve()
            o,e,rc,_=run([t,'env',f'AHC008_BRIDGE_IDLE_LOG={log}',f'AHC008_BANK_LOG={geo}',a.solver.resolve()],data,3.)
            assert rc==0 and e.strip()==('Score = '+rr['score']).encode()
            assert hashlib.sha256(o).hexdigest()==rr['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([t,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,trace],data,10.)
            assert (po,pe,pr)==(o,e,0)
            tape=trace.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(data.decode(),tape,cmds,detailed=True)[0]==int(rr['score'])
            _,hist,_=observed(data,tape);rows.extend(examine(name,log,geo,hist,cmds))
        print(name,flush=True)
    full=[r for r in rows if r['all_ready']]
    strict=[r for r in rows if r['route_open'] and r['reserve_nonconflict'] and r['donor_stays'] and not r['ordinary_builds']]
    enough=lambda rr:dict(candidates=len(rr),cases=len({r['case'] for r in rr}),
                          independent_case_turns=len({(r['case'],r['turn']) for r in rr}),
                          parking=sum(r['actual_parking'] for r in rr))
    s=dict(verified=len(refs),activated_full_scheduling=enough(rows),all_braces_ready=enough(full),
           hindsight_preserve_all=enough(strict),
           hindsight_preserve_all_fully_ready=enough([r for r in strict if r['all_ready']]))
    a.out.write_text(''.join(json.dumps(x,separators=(',',':'))+'\n' for x in rows))
    a.summary.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))
if __name__=='__main__':main()
