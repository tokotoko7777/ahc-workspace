#!/usr/bin/env python3
"""Reconstruct every allocation, move and fee from the official tester transcript."""
import argparse
import heapq
import hashlib
import json
import math
from pathlib import Path
import re
import subprocess
import time


def round_positive(x):
    v=int(x)
    return v+int(x-v>=.5)


def replay(data,answer):
    lines=data.splitlines()
    n,m,r=lines[0].split()
    n,m,r=int(n),int(m),float(r)
    board=[[ch!=46 for ch in row] for row in lines[1:n+1]]
    groups=[list(map(int,line.split())) for line in lines[n+1:n+1+m]]
    if len(groups)!=m: raise ValueError("groups missing")
    out=iter(answer.split())
    active={}
    ends=[]
    revenue=moves=accepted=0

    def fee(j,worst):
        p,v=groups[j][3:]
        return round_positive(v*(4*math.sqrt(p)/worst))

    def region(j):
        cells=set()
        p=groups[j][3]
        for _ in range(p):
            x,y=int(next(out)),int(next(out))
            if not (0<=x<n and 0<=y<n) or board[x][y] or (x,y) in cells:
                raise ValueError("invalid region cell")
            cells.add((x,y))
        visit={next(iter(cells))}
        queue=list(visit)
        boundary=0
        for x,y in queue:
            for a,b in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)):
                if (a,b) not in cells:
                    boundary+=1
                elif (a,b) not in visit:
                    visit.add((a,b))
                    queue.append((a,b))
        if len(visit)!=p:
            raise ValueError("disconnected region")
        for x,y in cells: board[x][y]=True
        return cells,boundary

    for i,current in enumerate(groups):
        j,start,end,p,value=current
        if j!=i: raise ValueError("input order")
        while ends and ends[0][0]<start:
            _,z=heapq.heappop(ends)
            cells,l=active.pop(z)
            for x,y in cells: board[x][y]=False
            revenue+=fee(z,l)
        moves_n=int(next(out))
        if not (0<=moves_n<=len(active)): raise ValueError("move count")
        # Coordinates are read as each group's record; legality, however, is
        # checked after everyone in the batch has vacated.
        plans=[]
        seen=set()
        for _ in range(moves_n):
            z=int(next(out))
            if z not in active or z in seen: raise ValueError("inactive/repeated mover")
            seen.add(z)
            coords=[next(out) for _ in range(2*groups[z][3])]
            plans.append((z,coords))
        for z,_ in plans:
            for x,y in active[z][0]: board[x][y]=False
        remainder=out
        for z,coords in plans:
            out=iter(coords)
            cells,perim=region(z)
            active[z]=(cells,max(active[z][1],perim))
            revenue-=max(1,round_positive(groups[z][4]*r))
            moves+=1
        out=remainder
        decision=next(out)
        if decision==b"Yes":
            active[i]=region(i)
            heapq.heappush(ends,(end,i))
            accepted+=1
        elif decision!=b"No":
            raise ValueError("admission token")
    if next(out,None) is not None:
        raise ValueError("extra tokens")
    return {"score":max(0,revenue+sum(fee(j,l) for j,(_,l) in active.items())),
            "moved":moves,"accepted":accepted}

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def evaluate(case, tester, solver):
    data=case.read_bytes()
    start=time.perf_counter()
    result=subprocess.run([tester,solver],input=data,capture_output=True,check=True,timeout=2)
    elapsed=time.perf_counter()-start
    if elapsed>=2:
        raise TimeoutError(case)
    independent=replay(data,result.stdout)
    official=re.findall(rb"^Score = (\d+)$",result.stderr,re.M)
    own=re.findall(rb"^Raw score = (\d+)$",result.stderr,re.M)
    if len(official)!=1 or len(own)!=1 or int(official[0])!=independent["score"] or int(own[0])!=independent["score"]:
        raise ValueError((case,solver,independent,result.stderr[-1000:]))
    return {**independent,"wall":elapsed,
            "protocol":hashlib.sha256(result.stdout).hexdigest()}


def main():
    p=argparse.ArgumentParser()
    for label in ("input_dir","output_dir","tester","control","candidate"):
        p.add_argument(label,type=lambda p: Path(p).resolve())
    a=p.parse_args()
    a.output_dir.mkdir(parents=True,exist_ok=True)
    files=sorted(a.input_dir.glob("*.txt"))
    assert files, "no inputs"
    solvers={"control":a.control,"candidate":a.candidate}
    manifest={"runner":sha(Path(__file__)),"tester":sha(a.tester),
              "binaries":{k:sha(v) for k,v in solvers.items()},"inputs":{p.name:sha(p) for p in files}}
    meta=a.output_dir/"manifest.json"
    if meta.exists() and json.loads(meta.read_text())!=manifest:
        raise ValueError("stale checkpoints")
    meta.write_text(json.dumps(manifest,indent=2)+"\n")
    rows={k:[] for k in solvers}
    for i,case in enumerate(files):
        for k in (solvers if i%2==0 else reversed(solvers)):
            path=a.output_dir/f"{case.stem}-{k}.json"
            if path.exists():
                r=json.loads(path.read_text())
            else:
                r=evaluate(case,a.tester,solvers[k])
                tmp=path.with_suffix(".tmp")
                tmp.write_text(json.dumps(r)+"\n")
                tmp.replace(path)
            rows[k].append(r)
    summary={k:{"legal":len(r),"score":sum(s["score"] for s in r),
                "max_wall":max(s["wall"] for s in r),
                "accepted":sum(s["accepted"] for s in r),"moved":sum(s["moved"] for s in r)} for k,r in rows.items()}
    scores=[y["score"]-x["score"] for x,y in zip(rows["control"],rows["candidate"])]
    summary.update(wins=sum(x>0 for x in scores),losses=sum(x<0 for x in scores),
                   ties=sum(x==0 for x in scores))
    (a.output_dir/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary),flush=True)


if __name__=="__main__":
    main()
