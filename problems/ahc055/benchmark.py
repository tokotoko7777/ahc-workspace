#!/usr/bin/env python3
"""Replay every opening, unlock, and charge independently."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def score(case,output):
    n,*data=map(int,case.split())
    hardness=data[:n]
    charge=data[n:2*n]
    matrix=data[2*n:]
    if len(matrix)!=n*n:
        raise ValueError("invalid damage table")
    initial=sum(hardness)
    proc=[]; turns=0
    for row in output.splitlines():
        w,b=map(int,row.split())
        if not (-1<=w<n and 0<=b<n) or hardness[b]<=0:
            raise ValueError("invalid target or weapon")
        turns+=1
        if w>=0:
            if hardness[w]>0 or charge[w]<=0:
                raise ValueError("unavailable or exhausted weapon")
            charge[w]-=1
            value=matrix[w*n+b]
        else:
            value=1
        hardness[b]-=value
        if hardness[b]<=0:
            proc.append(b)
    if len(proc)!=n:
        raise ValueError("unopened box")
    return {"score":initial-turns+1,"turns":turns,
            "order":hashlib.sha256(repr(proc).encode()).hexdigest()}

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser()
    for name in ("inputs", "out", "vis", "baseline", "candidate"):
        p.add_argument(name,type=Path)
    p.add_argument("--allow-order-change", action="store_true")
    args = p.parse_args()
    files = sorted(args.inputs.resolve().glob("*.txt"))
    out = args.out.resolve()
    out.mkdir(parents=True,exist_ok=True)
    solvers = {"baseline":args.baseline.resolve(),"allocated":args.candidate.resolve()}
    vis = args.vis.resolve()
    manifest = out/"manifest.json"
    metadata = {"allow_order_change":args.allow_order_change,"runner":sha(Path(__file__)),"vis":sha(vis),
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
                if elapsed >= 2:
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
        if not args.allow_order_change and results[case.name,"baseline"]["order"]!=results[case.name,"allocated"]["order"]:
            raise ValueError("open order changed; expected only allocation to differ")
    summary={}
    for key in solvers:
        rows=[results[c.name,key] for c in files]
        summary[key]={"valid":len(rows),"total":sum(r["score"] for r in rows),
                      "turns":sum(r["turns"] for r in rows),
                      "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"]=sum(results[c.name,"allocated"]["score"]>results[c.name,"baseline"]["score"]
                                  for c in files)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
