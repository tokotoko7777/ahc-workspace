"""Optimistic sterile-room area available to a separate protected spine."""
import argparse,csv,re,sys,tempfile
from pathlib import Path
from collective_cut_audit import layout,parts,TIMES
from guarded_entry_trace import observed
from collective_baffle_audit import start_time

def measure(hist):
    r,portals,adj,rooms=layout(4,4)
    stop,_=start_time(hist)
    best_single=best_sum=best_gap=0
    at_single=at_sum=0
    times=[t for t in TIMES if t<stop or stop<0]
    for t in times:
        peas,humans,walls=hist[t]
        occupied=set()
        for p in peas:
            if p in r:occupied.add(r[p])
            if p in portals:occupied.update(portals[p])
        clean=set(range(16))-occupied
        area=[len(x-walls) for x in rooms]
        union=max((sum(area[i] for i in group) for group in parts(clean,adj)),default=0)
        both=sum(area[i] for i in clean)
        if union>best_single:best_single,at_single=union,both
        if both>best_sum:best_sum,at_sum=both,union
        best_gap=max(best_gap,both-union)
    return [stop,len(times),best_single,best_sum,best_sum-best_single,
            at_single-best_single,best_sum-at_sum,best_gap]

def measure_service(hist):
    # Fixed one-wide row 14 service spine. Side retaining walls at rows
    # 13 and 15, each with a portal to all three lobes. Two ribs per side
    # at columns 10,20; three as-yet-open tip cells per rib.
    spine={(14,c) for c in range(30)}
    rooms=[{(r,c) for r in rr for c in cc}
           for rr in (range(13),range(16,30)) for cc in (range(10),range(11,20),range(21,30))]
    gates=[(r,c) for r in (13,15) for c in (4,15,25)]
    stop,_=start_time(hist)
    best=clear_spine_best=0
    for t in TIMES:
        if 0<=stop<=t:continue
        ps,hs,walls=hist[t]
        animals=set(ps)
        # No side/rib-portal occupancy, possible obstruction, wall setup,
        # loss to existing walls, or travel constraints charged. This is a
        # fixed-layout projection, not an altered official simulation.
        fertile=sum(len(room) for room,gate in zip(rooms,gates) if not (room&animals) and gate not in animals)
        best=max(best,fertile)
        if not animals&spine:clear_spine_best=max(clear_spine_best,fertile)
    return [stop,best,clear_spine_best]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ("harness","tester","solver","cases"):p.add_argument(key,type=Path)
    p.add_argument("--service-oracle",action="store_true")
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester,solver=a.tester.resolve(),a.solver.resolve()
    script=a.harness.resolve()/"benchmark_cases.py"
    out=csv.writer(sys.stdout,lineterminator="\n")
    out.writerow(["case","seal_turn","best_service_lobes","best_also_empty_spine"] if a.service_oracle else ["case","seal_turn","samples","best_connected","best_sterile_sum","separate_peak_gain",
                  "same_turn_gain_at_best_connected","same_turn_gain_at_best_sum","largest_same_turn_gap"])
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
        out.writerow([f.name,*(measure_service(h) if a.service_oracle else measure(h))])
        sys.stdout.flush()
if __name__=="__main__":main()
