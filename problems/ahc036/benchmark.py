#!/usr/bin/env python3
"""Check the trip, contiguous signal copies and green signals independently."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def score(case, output):
    data=list(map(int,case.split()))
    n,m,t,la,lb=data[:5]
    adj=[set() for _ in range(n)]
    for a,b in zip(data[5:5+2*m:2],data[6:5+2*m:2],strict=True):
        adj[a].add(b)
        adj[b].add(a)
    target=data[5+2*m:5+2*m+t]
    rows=output.splitlines()
    A=list(map(int,rows[0].split()))
    if len(A)!=la or any(not 0<=v<n for v in A):
        raise ValueError("bad A source")
    if len(rows)-1>100000:
        raise ValueError("too many operations")
    B=[-1]*lb
    node=signals=turns=met=0
    if target[met]==node:
        met+=1
    for ln in rows[1:]:
        tag,*values=ln.split()
        items=list(map(int,values))
        if tag==b"m" and len(items)==1:
            v,=items
            if v not in adj[node] or v not in B:
                raise ValueError("forbidden road or red signal")
            turns+=1
            node=v
            if met<t and target[met]==node:
                met+=1
        elif tag==b"s" and len(items)==3:
            length,a,b=items
            if not (1<=length<=lb and 0<=a<=la-length and 0<=b<=lb-length):
                raise ValueError("out-of-range signal slice")
            B[b:b+length]=A[a:a+length]
            signals+=1
        else:
            raise ValueError("invalid instruction")
    if met!=t:
        raise ValueError("not all required cities were visited")
    return {"score":signals,"moves":turns}

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
    solvers = {"baseline":args.baseline.resolve(),"blocks":args.candidate.resolve()}
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
                                          capture_output=True,check=True,timeout=25)
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
                      "moves":sum(r["moves"] for r in rows),
                      "max_actions":max(r["score"]+r["moves"] for r in rows),
                      "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"]=sum(results[c.name,"blocks"]["score"]<results[c.name,"baseline"]["score"]
                                  for c in files)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
