"""Pair high-confidence lock states with the actual actions in their baseline traces."""
import argparse,csv,re,sys,tempfile
from collections import defaultdict
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,flood
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','solver','cases','candidates'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester,solver=a.tester.resolve(),a.solver.resolve()
    script=a.harness.resolve()/'benchmark_cases.py'
    groups=defaultdict(list)
    for x in csv.DictReader(a.candidates.open()):
        if float(x['guard4'])>=.8:groups[x['case']].append(x)
    out=csv.writer(sys.stdout,lineterminator='\n')
    out.writerow(['case','turn','animal','missing','guard4','area_now','actual_compact_by4','actual_min_pen_area_with_no_human_by4','closest_same_gate_built_within4','all_same_gates_built_within4','actual_final_pen_area_if_no_humans','eventually_compact'])
    for case,witnesses in sorted(groups.items()):
        casefile=a.cases/case
        payload=casefile.read_bytes()
        cmd,err,rc,_=run([tester,solver],payload,3.)
        match=re.fullmatch(rb'Score = (\d+)\s*',err)
        assert rc==0 and match
        with tempfile.TemporaryDirectory() as td:
            actions=Path(td)/'actions';actions.write_bytes(cmd)
            trace=Path(td)/'trace'
            actual,e,c,_=run([tester,sys.executable,script,'--playback',actions,trace],payload,10.)
            assert c==0 and actual==cmd and e==err
            data=trace.read_text().splitlines()
        score,*_=replay(payload.decode(),data,cmd.decode().splitlines(),detailed=True)
        assert score==int(match[1])
        _,hist,_=observed(payload,data)
        for x in witnesses:
            t=int(x['t']);i=int(x['animal'])
            goal=set(map(int,x['gates'].split('+')))
            amin=V=901; bu=-1; complete=False
            for dt in range(1,5):
                ps,hs,ws=hist[t+dt]
                wp=set(map(idx,ws))
                if bu<0 and goal&wp:bu=dt
                complete|=goal<=wp
                reach=flood(B[idx(ps[i])],ALL^sum(B[w] for w in wp))
                if not any(reach&B[idx(h)] for h in hs):amin=min(amin,reach.bit_count())
            endsize=-1
            ever=amin<=24
            for k in range(t+5,301):
                ps,hs,ws=hist[k]
                if ever and k<300:continue
                realm=flood(B[idx(ps[i])],ALL^sum(B[idx(w)] for w in ws))
                alone=not any(realm&B[idx(h)] for h in hs)
                if alone and realm.bit_count()<=24:ever=True
                if k==300 and alone:endsize=realm.bit_count()
            out.writerow([case,t,i,x['missing'],x['guard4'],x['area'],int(amin<=24),amin if amin<V else -1,bu,int(complete),endsize,int(ever)])
if __name__=='__main__':main()
