#!/usr/bin/env python3
"""Independently simulate crane motion, boarding and correct-door inversions."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def score(case,output):
    data=list(map(int,case.split()))
    n=data[0]
    if n!=5 or sorted(data[1:])!=list(range(n*n)):
        raise ValueError("bad input permutation")
    orders=[data[1+i*n:1+(i+1)*n] for i in range(n)]
    m=[0]*n
    lines=output.decode("ascii").splitlines()
    if len(lines)!=n or not all(1<=len(x)<=10000 for x in lines):
        raise ValueError("bad action line count or length")
    tmax=max(map(len,lines))
    crates={}
    cranes=[(i,0) for i in range(n)]
    held=[None]*n
    outbound=[[] for _ in range(n)]
    dirs=dict(U=(-1,0),D=(1,0),L=(0,-1),R=(0,1))
    for t in range(tmax):
        previous=cranes[:]
        for r in range(n):
            inlet=(r,0)
            if (inlet not in crates and m[r]<n and all(
                    pos!=inlet or cargo is None for pos,cargo in zip(cranes,held,strict=True))):
                crates[inlet]=orders[r][m[r]]
                m[r]+=1
        for k,line in enumerate(lines):
            c=line[t] if t<len(line) else "."
            pos=previous[k]
            if c==".":
                continue
            if pos is None:
                raise ValueError("moving a destroyed crane")
            if c=="P" and held[k] is None and pos in crates:
                held[k]=crates.pop(pos)
            elif c=="Q" and held[k] is not None and pos not in crates:
                crates[pos]=held[k]
                held[k]=None
            elif c=="B" and held[k] is None:
                cranes[k]=None
            elif c in dirs:
                x,y=dirs[c]
                r,j=pos[0]+x,pos[1]+y
                if not (0<=r<n and 0<=j<n) or (k and held[k] is not None and (r,j) in crates):
                    raise ValueError("blocked or out of bounds")
                cranes[k]=(r,j)
            else:
                raise ValueError("invalid action or container state")
        occupied=[c for c in cranes if c is not None]
        if len(occupied)!=len(set(occupied)):
            raise ValueError("same-cell collision")
        for k,v in enumerate(cranes):
            if v is None:
                continue
            for j in range(k):
                if cranes[j] is not None and previous[j]==v and previous[k]==cranes[j]:
                    raise ValueError("edge swap collision")
        for r in range(n):
            end=(r,n-1)
            if end in crates:
                outbound[r].append(crates.pop(end))
    wrong=inversions=shipped=0
    for r,group in enumerate(outbound):
        shipped+=len(group)
        proper=[a for a in group if a//n==r]
        wrong+=len(group)-len(proper)
        inversions+=sum(a>b for i,a in enumerate(proper) for b in proper[i+1:])
    missing=n*n-shipped
    return {"score":tmax+100*inversions+10000*wrong+1000000*missing,
            "turns":tmax,"inversions":inversions,"wrong":wrong,"missing":missing}

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
    solvers = {"baseline":args.baseline.resolve(),"parallel":args.candidate.resolve()}
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
                      "inversions":sum(r["inversions"] for r in rows),
                      "missing":sum(r["missing"] for r in rows),
                      "wrong":sum(r["wrong"] for r in rows),
                      "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"]=sum(results[c.name,"parallel"]["score"]<results[c.name,"baseline"]["score"]
                                  for c in files)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
