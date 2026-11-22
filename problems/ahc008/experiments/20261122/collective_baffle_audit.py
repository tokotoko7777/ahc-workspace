"""Retrospective baffle annex test immediately before capture4 first clears."""
import argparse,csv,re,sys,tempfile
from pathlib import Path
from collective_cut_audit import layout,can_staff,BOARD,N
from guarded_entry_trace import observed

def flood(starts,allowed):
    seen=set(starts)&allowed
    q=list(seen)
    for p in q:
        for n in N[p]:
            if n in allowed and n not in seen:
                seen.add(n);q.append(n)
    return seen

def start_time(hist):
    def clear(t):
        p,h,w=hist[t]
        opened=flood(h,BOARD-w)
        return not (opened&set(p))
    if not clear(300):return -1,set()
    a,b=0,300
    while a<b:
        m=(a+b)//2
        if clear(m):b=m
        else:a=m+1
    _,humans,walls=hist[a]
    return a,flood(humans,BOARD-walls)

def discover(hist):
    t,base=start_time(hist)
    if t<=0:return [-1,0,0,0,0,0,0,0]
    pets,people,walls=hist[t-1]
    animals=set(pets)
    forbidden=animals|set(people)|{n for p in animals for n in N[p]}
    if base&animals:
        raise RuntimeError("Permanent final clear region must be initially animal-free")
    region,portals,adj,cells=layout(4,4)
    initial={n for p in base for n in N[p] if n not in base and n not in walls}
    candidates={}
    best=[0,0,0,0,0]
    for i,tile in enumerate(cells):
        if not animals&tile:continue
        proposals=[]
        for axis in (0,1):
            for d in range(min(p[axis] for p in tile)+1,max(p[axis] for p in tile)):
                for sign in (-1,1):
                    half={p for p in tile if sign*(p[axis]-d)>0 and p not in walls}
                    if not half or animals&half:continue
                    join={p for p,q in portals.items() if i in q and p not in walls and
                          any(a in base for a in N[p]) and any(a in half for a in N[p]) and p not in animals}
                    if not join:continue
                    extra=flood(join,half|join)
                    domain=base|extra
                    final={n for p in domain for n in N[p] if n not in domain and n not in walls}
                    new=final-initial
                    gain=len(extra-base)
                    if not gain:continue
                    best[0]=max(best[0],gain)
                    if len(new)<=2:best[2]=max(best[2],gain)
                    if len(new)<=2 and not final&forbidden:
                        for n in (2,0):
                            if can_staff(final,people,n,walls,domain):
                                best[3+int(n==0)]=max(best[3+int(n==0)],gain)
                    proposals.append((gain,extra))
        if proposals:candidates[i]=max(proposals,key=lambda x:x[0])[1]
    if candidates:
        merged=base|set().union(*candidates.values())
        best[1]=len(merged-base)
    return [t,len(base),len(initial),*best]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("harness",type=Path);p.add_argument("tester",type=Path);p.add_argument("solver",type=Path);p.add_argument("cases",type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester,solver=a.tester.resolve(),a.solver.resolve()
    harness=a.harness.resolve()/"benchmark_cases.py"
    out=csv.writer(sys.stdout,lineterminator="\n")
    out.writerow(["case","first_all_clear","old_safe_area","old_boundary_builds","optimistic_single_gain","optimistic_multi_gain",
                  "single_gain_at_most_2_added_walls","gain_2moves_every_person","gain_no_moves_every_person"])
    for f in sorted(a.cases.glob("*.txt")):
        case=f.read_bytes()
        commands,err,rc,_=run([tester,solver],case,3.)
        match=re.fullmatch(rb"Score = (\d+)\s*",err)
        assert rc==0 and match
        with tempfile.TemporaryDirectory() as td:
            actions=Path(td)/"actions";actions.write_bytes(commands)
            trace=Path(td)/"trace"
            ans,e,code,_=run([tester,sys.executable,harness,"--playback",actions,trace],case,10.)
            assert code==0 and ans==commands and e==err
            data=trace.read_text().splitlines()
        res,*_=replay(case.decode(),data,commands.decode().splitlines(),detailed=True)
        assert res==int(match[1])
        _,history,_=observed(case,data)
        out.writerow([f.name,*discover(history)])
        sys.stdout.flush()
if __name__=="__main__":main()
