"""Find conditional local cut opportunities on verified capture4 trajectories."""
import argparse
import csv
from collections import defaultdict
import re
from pathlib import Path
import sys
import tempfile

STEPS = dict(zip("UDLR",[(-1,0),(1,0),(0,-1),(0,1)]))
NEI = list(STEPS.values())

def step(p,d):
    return p[0]+d[0],p[1]+d[1]

def inboard(p):
    return 0<=p[0]<30 and 0<=p[1]<30

def neigh(p):
    return [x for d in NEI if inboard(x:=step(p,d))]

def footprints():
    """The no-dog 4x4 scaffold, anchored 2x3/3x2 corner footprints."""
    starts=(0,8,16,23)
    ends=(6,14,21,29)
    shapes=[]
    for i in range(4):
        for j in range(4):
            for sx in (1,-1):
                for sy in (1,-1):
                    for wd,dp in ((2,3),(3,2)):
                        x=starts[i] if sx==1 else ends[i]
                        y=starts[j] if sy==1 else ends[j]
                        cells=frozenset((x+sx*u,y+sy*v) for u in range(wd) for v in range(dp))
                        boundary=set(q for p in cells for q in neigh(p) if q not in cells)
                        supports={p for p in boundary if not (starts[i]<=p[0]<=ends[i] and starts[j]<=p[1]<=ends[j])}
                        shapes.append((cells,boundary,supports))
    starts_at=defaultdict(list)
    for cells,boundary,supports in shapes:
        for p in cells:
            starts_at[p].append((cells,boundary,supports))
    return starts_at

def observed(case, trace):
    source=iter(case.decode().splitlines())
    pets_data=[list(map(int,next(source).split())) for _ in range(int(next(source)))]
    kinds=[r[2] for r in pets_data]
    pets=[(r-1,c-1) for r,c,_ in pets_data]
    humans=[tuple(int(z)-1 for z in next(source).split()) for _ in range(int(next(source)))]
    walls=set()
    history=[(tuple(pets),tuple(humans),frozenset(walls))]
    entries=[]
    starts=footprints()
    for t in range(300):
        actions=trace[2*t]
        for i,c in enumerate(actions):
            if c!='.':
                u=step(humans[i],STEPS[c.upper()])
                if c.islower():walls.add(u)
                else:humans[i]=u
        for i,cmd in enumerate(trace[2*t+1].split()):
            for c in cmd:
                if c=='.':continue
                old=pets[i]
                new=step(old,STEPS[c])
                for cells,boundary,supports in starts.get(new,()):
                    if old in boundary:
                        entries.append((t+1,i,old,cells,boundary,supports))
                pets[i]=new
        history.append((tuple(pets),tuple(humans),frozenset(walls)))
    return kinds,history,entries

def count_metrics(kinds,history,entries,stop):
    out=[[0]*9 for k in kinds]
    for initial,i,gate,cells,boundary,supports in entries:
        if initial>=stop:continue
        out[i][0]=1
        hypothetical=boundary-{gate}
        forbidden=set(neigh(gate))|{gate}
        stances=set(neigh(gate))-cells-hypothetical
        for t in range(initial,min(stop,initial+5)):
            pets,people,wall=history[t]
            p=pets[i]
            if p not in cells and p!=gate:
                break
            if p not in cells or p in forbidden or gate in wall:
                continue
            out[i][1]=1
            if any(p in forbidden for p in pets) or gate in people:continue
            out[i][2]=1
            spots=stances-wall
            near=set(q for p in people for q in neigh(p) if q not in wall)
            adjacent=bool(set(people)&spots)
            one=adjacent or bool(near&spots)
            out[i][3]|=adjacent
            out[i][4]|=one
            if not (supports-{gate})-wall:
                out[i][7]=1
                out[i][8]|=one
            if not hypothetical-wall:
                out[i][5]=1
                out[i][6]|=one
    return out

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("harness",type=Path)
    p.add_argument("tester",type=Path)
    p.add_argument("solver",type=Path)
    p.add_argument("input_dir",type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester,solver=a.tester.resolve(),a.solver.resolve()
    harness=a.harness.resolve()/"benchmark_cases.py"
    out=csv.writer(sys.stdout,lineterminator="\n")
    out.writerow(["case","species","count","entered","potential_target_safe","all_pets_legal",
                  "currently_staffed","guard_within_one","actually_prepared","actual_and_guarded",
                  "macro_anchor_ready","macro_anchor_and_guarded"])
    for f in sorted(a.input_dir.glob("*.txt")):
        case=f.read_bytes()
        actions,err,rc,_=run([tester,solver],case,3.)
        match=re.fullmatch(rb"Score = (\d+)\s*",err)
        assert rc==0 and match
        with tempfile.TemporaryDirectory() as td:
            commands=Path(td)/"recorded"
            trace=Path(td)/"trace"
            commands.write_bytes(actions)
            o,e,c,_=run([tester,sys.executable,harness,"--playback",commands,trace],case,10.)
            assert c==0 and o==actions and e==err
            lines=trace.read_text().splitlines()
        checks=[]
        def collect(turn,groups):
            checks.append((turn,not any(sum(k) for n,_,k in groups if n)))
        actual,*_=replay(case.decode(),lines,actions.decode().splitlines(),detailed=True,snapshots=collect,interval=10)
        assert actual==int(match[1])
        seal=min((t for j,(t,_) in enumerate(checks) if all(c for _,c in checks[j:])),default=300)
        kinds,hist,entries=observed(case,lines)
        flags=count_metrics(kinds,hist,entries,min(297,seal))
        for kind in range(1,6):
            sn=[row for k,row in zip(kinds,flags) if k==kind]
            out.writerow([f.name,kind,len(sn),*[sum(x[i] for x in sn) for i in range(9)]])
        sys.stdout.flush()
if __name__=="__main__":main()
