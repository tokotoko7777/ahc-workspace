#!/usr/bin/env python3
"""Replay fixed wells, color mixtures, paint amounts, and score independently."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def score(case, output):
    numbers=case.split()
    n,k,h,t,d=map(int,numbers[:5])
    values=list(map(float,numbers[5:]))
    colors=list(zip(values[::3],values[1::3],values[2::3],strict=True))
    if len(colors)!=k+h:
        raise ValueError("invalid paint instance")
    tube,target=colors[:k],colors[k:]
    raw=output.splitlines()
    walls=list(map(lambda row:list(map(int,row.split())),raw[:2*n-1]))
    if (len(walls)!=2*n-1 or
            any(len(row)!=(n-1 if i<n else n) for i,row in enumerate(walls)) or
            any(x not in (0,1) for row in walls for x in row)):
        raise ValueError("invalid divider shape")
    parent=list(range(n*n))
    def root(v):
        while v!=parent[v]:
            v=parent[v]
        return v
    for i in range(n):
        for j,b in enumerate(walls[i]):
            if not b:
                parent[root(i*n+j)]=root(i*n+j+1)
    for i in range(n-1):
        for j,b in enumerate(walls[n+i]):
            if not b:
                parent[root(i*n+j)]=root((i+1)*n+j)
    cap=[0]*(n*n)
    for v in range(n*n):
        cap[root(v)]+=1
    mass=[0]*(n*n)
    mixtures=[(0.,0.,0.)]*(n*n)
    errors=0.
    made=pours=0
    ops=raw[2*n-1:]
    if len(ops)>t:
        raise ValueError("too many operations")
    for row in ops:
        code,r,c,*tail=map(int,row.split())
        if not (0<=r<n and 0<=c<n):
            raise ValueError("invalid paint cell")
        v=root(r*n+c)
        if code==1 and len(tail)==1 and 0<=tail[0]<k:
            gram=min(cap[v]-mass[v],1)
            count=mass[v]+gram
            mixtures[v]=tuple((old*mass[v]+new*gram)/count
                              for old,new in zip(mixtures[v],tube[tail[0]],strict=True))
            mass[v]=count
            pours+=1
        elif code==2 and not tail and made<h and mass[v]>=1:
            errors+=sum((a-b)*(a-b) for a,b in zip(mixtures[v],target[made],strict=True))**.5
            made+=1
            mass[v]-=1
        elif code==3 and not tail:
            mass[v]=max(0,mass[v]-1)
        else:
            raise ValueError("bad paint operation or insufficient paint; static dividers only")
    if made!=h:
        raise ValueError("wrong delivery count")
    cost=1+d*(pours-h)+int(1e4*errors+0.5)
    return {"score":cost,"pours":pours,"ops":len(ops)}

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
    solvers = {"baseline":args.baseline.resolve(),"dilution":args.candidate.resolve()}
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
                      "pours":sum(r["pours"] for r in rows),
                      "max_ops":max(r["ops"] for r in rows),
                      "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"]=sum(results[c.name,"dilution"]["score"]<results[c.name,"baseline"]["score"]
                                  for c in files)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
