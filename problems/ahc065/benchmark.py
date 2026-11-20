#!/usr/bin/env python3
"""Validate installed cycles and replay the simultaneous conveyor moves."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import subprocess
import time


def replay(case, output):
    n,*board=map(int,case.split())
    ops=iter(map(int,output.split()))
    belts=[]
    visits=[0]*(n*n)
    count=next(ops)
    if not 0<=count<=n*n:
        raise ValueError("belt number")
    for _ in range(count):
        k=next(ops)
        pts=[(next(ops),next(ops)) for _ in range(k)]
        if k<2 or len(set(pts))!=k:
            raise ValueError("cycle is too small or repeats a cell")
        for i,(x,y) in enumerate(pts):
            a,b=pts[i-1]
            if not (0<=x<n and 0<=y<n) or abs(a-x)+abs(b-y)!=1:
                raise ValueError("invalid cyclic step")
            visits[x*n+y]+=1
        belts.append([x*n+y for x,y in pts])
    if max(visits)>2:
        raise ValueError("more than two cycles through a cell")
    turns=next(ops)
    if not 0<=turns<=10**5:
        raise ValueError("operation limit")
    door=n//2
    delivered=(board[door]==0)
    if delivered:
        board[door]=-1
    for _ in range(turns):
        chosen,d=next(ops),next(ops)
        if not 0<=chosen<len(belts) or d not in (-1,1):
            raise ValueError("invalid operation")
        cycle=belts[chosen]
        old=[board[x] for x in cycle]
        for i,v in enumerate(old):
            board[cycle[(i+d)%len(old)]]=v
        if board[door]==delivered:
            delivered+=1
            board[door]=-1
    if next(ops,None) is not None:
        raise ValueError("extra output")
    score=(10**6+math.floor(10**6*math.log2(10**5/turns)+.5)
           if delivered==n*n else math.floor(10**6*delivered/(n*n)+.5))
    if delivered!=n*n or any(x>=0 for x in board):
        raise ValueError("solution did not fully clear")
    return {"cost":turns,"score":score}


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser()
    for label in ("input_dir","output_dir","vis","control","candidate"):
        p.add_argument(label,type=lambda p: Path(p).resolve())
    a=p.parse_args()
    a.output_dir.mkdir(parents=True,exist_ok=True)
    files=sorted(a.input_dir.glob("*.txt"))
    assert files, "no inputs"
    solvers={"control":a.control,"candidate":a.candidate}
    manifest={"runner":sha(Path(__file__)),"vis":sha(a.vis),"binaries":{key:sha(x) for key,x in solvers.items()},
              "inputs":{p.name:sha(p) for p in files}}
    meta=a.output_dir/"manifest.json"
    if meta.exists() and json.loads(meta.read_text()) != manifest:
        raise ValueError("stale checkpoints")
    meta.write_text(json.dumps(manifest,indent=2)+"\n")
    rows={key:[] for key in solvers}
    scratch=a.output_dir/"solution.txt"
    for idx, case in enumerate(files):
        for key in (solvers if idx%2==0 else reversed(solvers)):
            row=a.output_dir/f"{case.stem}-{key}.json"
            if row.exists():
                rows[key].append(json.loads(row.read_text()))
                continue
            data=case.read_bytes()
            start=time.perf_counter()
            result=subprocess.run([solvers[key]],input=data,capture_output=True,check=True,timeout=2)
            wall=time.perf_counter()-start
            if wall>=2:
                raise TimeoutError(wall)
            summary=replay(data,result.stdout)
            internal=re.fullmatch(rb"Turns = (\d+)\s*",result.stderr)
            scratch.write_bytes(result.stdout)
            vis=subprocess.run([a.vis,case,scratch],cwd=a.output_dir,capture_output=True,check=True,timeout=10)
            official=re.search(rb"Score = (\d+)",vis.stdout+vis.stderr)
            if (not official or not internal or int(official[1]) != summary["score"]
                or int(internal[1]) != summary["cost"]):
                raise ValueError((case,key,summary,result.stderr,vis.stdout,vis.stderr))
            summary["wall"]=wall
            tmp=row.with_suffix(".tmp")
            tmp.write_text(json.dumps(summary)+"\n")
            tmp.replace(row)
            rows[key].append(summary)
    results={key:{"valid":len(series),"score":sum(s["score"] for s in series),
                  "cost":sum(s["cost"] for s in series),"max_wall":max(s["wall"] for s in series)} for key,series in rows.items()}
    results["wins"]=sum(a["score"]<b["score"] for a,b in zip(rows["control"],rows["candidate"]))
    results["losses"]=sum(a["score"]>b["score"] for a,b in zip(rows["control"],rows["candidate"]))
    (a.output_dir/"summary.json").write_text(json.dumps(results,indent=2)+"\n")
    print(json.dumps(results),flush=True)


if __name__ == "__main__":
    main()
