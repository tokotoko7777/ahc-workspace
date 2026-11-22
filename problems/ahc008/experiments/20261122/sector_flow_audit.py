"""Causal sector choices, followed by observed exits and preventable ingress."""
import argparse,csv,re,sys,tempfile
from pathlib import Path
from species_blocker_audit import ALL,LEFT,RIGHT,bit,components,candidate_rooms
from guarded_entry_trace import observed,STEPS,step

def wave(x):
    return (((x&~RIGHT)<<1)|((x&~LEFT)>>1)|(x<<30)|(x>>30))&ALL

def paths(initial,moves):
    p=initial
    seen=bit(p)
    for c in moves:
        if c!=".":
            p=step(p,STEPS[c])
            seen|=bit(p)
    return seen

def measure(domains,hist,trace):
    for t in (70,100,130):
        ps,hs,walls=hist[t]
        passable=ALL^sum(map(bit,walls))
        pbits=list(map(bit,ps))
        best=None
        for d in domains:
            for comp in components(d&passable):
                wants=wave(comp)&passable&~comp
                missing=wants.bit_count()
                if missing>12:continue
                own={i for i,p in enumerate(pbits) if comp&p}
                key=(len(own),missing,-comp.bit_count(),comp)
                if best is None or key<best[0]:best=(key,comp,wants,own)
        if best is None:
            yield [t]+[-1]*14
            continue
        _,comp,wants,own=best
        future=list(range(t,t+20))
        moves={k:[paths(p,m) for p,m in zip(hist[k][0],trace[2*k+1].split())] for k in future}
        exited={i for i in own if any(moves[k][i]&~comp for k in future)}
        outsiders=set(range(len(ps)))-own
        first={i:next((k for k in future if moves[k][i]&comp),None) for i in outsiders}
        incoming={i for i in outsiders if first[i] is not None}
        border=all_gates(wants)
        prepped=set();staffed=set();reachable2=set()
        for i in incoming:
            for g in border:
                gbit=1<<g
                ent=first[i]
                if not (gbit&moves[ent][i]):continue
                # Retain every observed initial blocker exit through this gate up to end of the horizon.
                for when in range(t,ent+1):
                    if any(moves[k][j]&gbit for k in range(when,t+20) for j in own):continue
                    pp,hh,ww=hist[when]
                    pb=0;hc=0
                    for p in pp:pb|=bit(p)
                    for h in hh:hc|=bit(h)
                    if gbit&(pb|wave(pb)|hc|sum(map(bit,ww))):continue
                    prepped.add(i)
                    guard=wave(gbit)&comp
                    if hc&guard:staffed.add(i)
                    walkable=ALL^sum(map(bit,ww))
                    reach=(hc|(wave(hc)&walkable))
                    reach|=wave(reach)&walkable
                    if reach&guard:reachable2.add(i)
        drained=any(not any(comp&bit(p) for p in hist[k][0]) for k in range(t+1,t+21))
        leave_turns=[next((k for k in future if moves[k][i]&~comp),None) for i in own]
        latest_exit=max((x for x in leave_turns if x is not None),default=-1)
        new_in_before_drain=sum(first[i]<=latest_exit for i in incoming)
        no_reentry=(len(exited)==len(own) and bool(own) and all(not (comp&bit(hist[t+20][0][i])) for i in own))
        yield [t,comp.bit_count(),wants.bit_count(),len(own),len(exited),len(incoming),
               len(prepped),len(staffed),len(reachable2),int(drained),latest_exit,new_in_before_drain,
               int(no_reentry),int(no_reentry and incoming<=prepped),int(no_reentry and incoming<=staffed)]

def all_gates(mask):
    while mask:
        x=mask&-mask
        yield x.bit_length()-1
        mask^=x

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ("harness","tester","solver","cases"):p.add_argument(key,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester,solver=a.tester.resolve(),a.solver.resolve()
    script=a.harness.resolve()/"benchmark_cases.py"
    domains=[sum(map(bit,d)) for d in candidate_rooms()]
    out=csv.writer(sys.stdout,lineterminator="\n")
    out.writerow(["case","turn","area","missing_walls","initial_blockers","blockers_cross_out","outsiders_cross_in",
                  "individual_ingress_legal","individual_ingress_actual_staff","individual_ingress_two_steps",
                  "observed_empty","last_first_blocker_exit","outsiders_enter_before_first_drain",
                  "all_blockers_end_out","all_end_out_and_individually_blockable",
                  "all_end_out_and_individually_actual_staff"])
    for f in sorted(a.cases.glob("*.txt")):
        case=f.read_bytes()
        cmd,err,rc,_=run([tester,solver],case,3.)
        match=re.fullmatch(rb"Score = (\d+)\s*",err)
        assert rc==0 and match
        with tempfile.TemporaryDirectory() as td:
            actions=Path(td)/"actions";actions.write_bytes(cmd)
            trace=Path(td)/"trace"
            actual,e,c,_=run([tester,sys.executable,script,"--playback",actions,trace],case,10.)
            assert c==0 and actual==cmd and e==err
            data=trace.read_text().splitlines()
        s,*_=replay(case.decode(),data,cmd.decode().splitlines(),detailed=True)
        assert s==int(match[1])
        _,h,_=observed(case,data)
        for row in measure(domains,h,data):out.writerow([f.name,*row])
        sys.stdout.flush()
if __name__=="__main__":main()
