#!/usr/bin/env python3
"""Recheck all parent edges and root paths independently."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def score(case,output):
    data=list(map(int,case.split()))
    n,m,cap=data[:3]
    w=data[3:3+n]
    edges=set()
    for a,b in zip(data[3+n:3+n+2*m:2],data[4+n:3+n+2*m:2],strict=True):
        edges.add((a,b))
        edges.add((b,a))
    parent=tuple(map(int,output.split()))
    if len(parent)!=n or any(p!=-1 and (v,p) not in edges for v,p in enumerate(parent)):
        raise ValueError("invalid parent edge or count")
    result=1
    for v,value in enumerate(w):
        k=0
        occupied={v}
        while parent[v]>=0:
            v=parent[v]
            k+=1
            if v in occupied or k>cap:
                raise ValueError("cycle or depth cap exceeded")
            occupied.add(v)
        result+=value*(k+1)
    return result

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
    solvers = {"baseline":args.baseline.resolve(),"reparent":args.candidate.resolve()}
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
    summary["candidate_wins"]=sum(results[c.name,"reparent"]["score"]>results[c.name,"baseline"]["score"]
                                  for c in files)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
