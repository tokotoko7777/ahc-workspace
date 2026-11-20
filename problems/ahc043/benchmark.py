#!/usr/bin/env python3
"""Rebuild rail connectivity and fare income from the exact turn actions."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def score(case,output):
    n,m,cash,turns,*locations=map(int,case.split())
    n2=n*n
    kinds=[-1]*n2
    pieces=[[] for _ in range(n2)]
    home=[[] for _ in range(m)]
    office=[[] for _ in range(m)]
    fares=[]
    for i,(r,c,a,b) in enumerate(zip(*(locations[k::4] for k in range(4)),strict=True)):
        fares.append(abs(r-a)+abs(c-b))
        for side,(x,y) in enumerate(((r,c),(a,b))):
            for dx in range(-2,3):
                for dy in range(-(2-abs(dx)),3-abs(dx)):
                    if 0<=x+dx<n and 0<=y+dy<n:
                        pieces[(x+dx)*n+y+dy].append((i,side))
    union=[-1]*n2
    def find(x):
        while union[x]>=0:
            parent=union[x]
            if union[parent]>=0:
                union[x]=union[parent]
            x=parent
        return x
    def join(a,b):
        a,b=find(a),find(b)
        if a!=b:
            if union[a]>union[b]:
                a,b=b,a
            union[a]+=union[b]
            union[b]=a
    links=(15,3,12,9,5,6,10)
    moves=output.splitlines()
    if len(moves)!=turns:
        raise ValueError("wrong turn count")
    live=[False]*m
    income=0
    for row in moves:
        op=list(map(int,row.split()))
        if op!=[-1]:
            if len(op)!=3:
                raise ValueError("invalid operation record")
            t,r,c=op
            if not (0<=t<=6 and 0<=r<n and 0<=c<n):
                raise ValueError("invalid construction")
            p=r*n+c
            if kinds[p]>=0 and (t or kinds[p]==0):
                raise ValueError("cannot overwrite")
            cash-=100 if t else 5000
            if cash<0:
                raise ValueError("negative cash before revenue")
            kinds[p]=t
            if t==0:
                for person,side in pieces[p]:
                    (office if side else home)[person].append(p)
            neigh=(p-1 if c else -1,p+1 if c+1<n else -1,
                   p-n if r else -1,p+n if r+1<n else -1)
            for d,q in enumerate(neigh):
                if q>=0 and kinds[q]>=0 and links[t]>>d&1 and links[kinds[q]]>>(d^1)&1:
                    join(p,q)
            for i in range(m):
                if not live[i] and home[i] and office[i]:
                    roots={find(x) for x in home[i]}
                    if any(find(x) in roots for x in office[i]):
                        income+=fares[i]
                        live[i]=True
        cash+=income
    return cash

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
    solvers = {"baseline":args.baseline.resolve(),"extensions":args.candidate.resolve()}
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
    summary["candidate_wins"]=sum(results[c.name,"extensions"]["score"]>results[c.name,"baseline"]["score"]
                                  for c in files)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
