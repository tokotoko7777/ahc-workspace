#!/usr/bin/env python3
"""Replay all simultaneous button presses and waxed cells independently."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def score(case,output):
    page=case.split()
    n,m,k=map(int,page[:3])
    p=list(map(int,page[3:3+2*m]))
    robots=list(zip(p[::2],p[1::2],strict=True))
    vertical=page[3+2*m:3+2*m+n]
    horizontal=page[3+2*m+n:]
    rows=output.splitlines()
    matrix=[tuple(x.decode("ascii") for x in s.split()) for s in rows[:k]]
    if (len(matrix)!=k or any(len(r)!=m for r in matrix)
            or any(x not in ("U","D","L","R","S") for row in matrix for x in row)):
        raise ValueError("invalid controller")
    presses=[int(row) for row in rows[k:]]
    if len(presses)>2*n*n or any(not 0<=a<k for a in presses):
        raise ValueError("invalid button or too many operations")
    visited=set(robots)
    for index in presses:
        nxt=[]
        for (r,c),direction in zip(robots,matrix[index],strict=True):
            if direction=="U" and r>0 and horizontal[r-1][c]==48:
                r-=1
            elif direction=="D" and r+1<n and horizontal[r][c]==48:
                r+=1
            elif direction=="L" and c>0 and vertical[r][c-1]==48:
                c-=1
            elif direction=="R" and c+1<n and vertical[r][c]==48:
                c+=1
            nxt.append((r,c))
        visited.update(nxt)
        robots=nxt
    covered=len(visited)
    return {"score":3*n*n-len(presses) if covered==n*n else covered,
            "turns":len(presses),"covered":covered,"complete":covered==n*n}

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
    solvers = {"baseline":args.baseline.resolve(),"individual":args.candidate.resolve()}
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
                if not internal or not official_value or actual["score"] != int(internal[1]) or actual["score"] != int(official_value[1]):
                    raise ValueError((case,name,actual,response.stderr,official.stdout,official.stderr))
                result = {**actual,"wall":elapsed}
                tmp = row.with_suffix(".tmp")
                tmp.write_text(json.dumps(result)+"\n")
                tmp.replace(row)
            results[case.name,name] = result
    summary={}
    for key in solvers:
        rows=[results[c.name,key] for c in files]
        summary[key]={"valid":len(rows),"total":sum(r["score"] for r in rows),
                      "turns":sum(r["turns"] for r in rows),
                      "complete":sum(r["complete"] for r in rows),
                      "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"]=sum(results[c.name,"individual"]["score"]>results[c.name,"baseline"]["score"]
                                  for c in files)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
