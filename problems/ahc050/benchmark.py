#!/usr/bin/env python3
"""Replay skating probability by aggregating rows and columns independently."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def score(case,output):
    header,*grid=case.split()
    n=int(header)
    count=int(grid.pop(0))
    initial=[b == 35 for s in grid for b in s]
    if len(initial)!=n*n or sum(initial)!=count:
        raise ValueError("malformed instance")
    actions=list(map(int,output.split()))
    left=n*n-count
    if len(actions)!=2*left:
        raise ValueError("wrong number of rocks")
    walls=initial.copy()
    curr=[0. if blocked else 1./left for blocked in walls]
    survival=1.
    total=0.
    for r,c in zip(actions[::2],actions[1::2],strict=True):
        if not 0<=r<n or not 0<=c<n or walls[r*n+c]:
            raise ValueError("rock is occupied or out of bounds")
        next_=[0.]*(n*n)
        # A left/right outcome from every free segment goes to the same
        # endpoint; sum the segment rather than following any cell's route.
        for rb in range(0,n*n,n):
            p=rb
            while p<rb+n:
                if walls[p]:
                    p+=1
                    continue
                a=p
                while p<rb+n and not walls[p]:
                    p+=1
                q=sum(curr[a:p])*.25
                next_[a]+=q
                next_[p-1]+=q
        for cb in range(n):
            p=cb
            while p<n*n:
                if walls[p]:
                    p+=n
                    continue
                a=p
                while p<n*n and not walls[p]:
                    p+=n
                q=sum(curr[a:p:n])*.25
                next_[a]+=q
                next_[p-n]+=q
        v=r*n+c
        survival-=next_[v]
        next_[v]=0.
        curr=next_
        total+=survival
        walls[v]=True
    return int(1e6*total/(left-1)+.5)

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser()
    for name in ("inputs", "out", "vis", "baseline", "candidate"):
        p.add_argument(name,type=Path)
    args = p.parse_args()
    files = sorted(args.inputs.resolve().glob("*.txt"))
    out = args.out.resolve()
    out.mkdir(parents=True,exist_ok=True)
    solvers = {"baseline":args.baseline.resolve(),"lookahead":args.candidate.resolve()}
    vis = args.vis.resolve()
    manifest = out/"manifest.json"
    metadata = {"runner":sha(Path(__file__)),"vis":sha(vis),
                "solvers":{k:sha(v) for k,v in solvers.items()},
                "inputs":{p.name:sha(p) for p in files}}
    if not files or manifest.exists() and json.loads(manifest.read_text()) != metadata:
        p.error("absent inputs or stale checkpoints")
    manifest.write_text(json.dumps(metadata,indent=2)+"\n")
    results = {}
    (out/"scratch").mkdir(exist_ok=True)
    for i,case in enumerate(files):
        for name in (solvers if i%2==0 else reversed(solvers)):
            row = out/f"{case.stem}-{name}.json"
            if row.exists():
                result = json.loads(row.read_text())
            else:
                inp = case.read_bytes()
                start = time.perf_counter()
                response = subprocess.run([solvers[name]],input=inp,capture_output=True,
                                          check=True,timeout=3)
                elapsed = time.perf_counter()-start
                if elapsed >= 3:
                    raise TimeoutError(f"{name} took {elapsed} s")
                actual = score(inp,response.stdout)
                internal = re.fullmatch(rb"Model = (\d+)\s*",response.stderr)
                target = out/"scratch"/"solver.out"
                target.write_bytes(response.stdout)
                official = subprocess.run([vis,case,target],cwd=out/"scratch",
                                          capture_output=True,check=True,timeout=10)
                official_value = re.search(rb"Score = (\d+)",official.stdout+official.stderr)
                if not internal or not official_value or actual != int(internal[1]) or actual != int(official_value[1]):
                    raise ValueError((case,name,actual,response.stderr,official.stdout,official.stderr))
                result = {"score":actual,"wall":elapsed}
                tmp = row.with_suffix(".tmp")
                tmp.write_text(json.dumps(result)+"\n")
                tmp.replace(row)
            results[case.name,name] = result
    summary={}
    for key in solvers:
        rows=[results[c.name,key] for c in files]
        summary[key]={"valid":len(rows),"total":sum(r["score"] for r in rows),
                      "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"]=sum(results[c.name,"lookahead"]["score"]>results[c.name,"baseline"]["score"]
                                  for c in files)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
