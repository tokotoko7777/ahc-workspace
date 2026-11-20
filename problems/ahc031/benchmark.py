#!/usr/bin/env python3
"""Check geometry and union-of-boundary cost independently of the C++ model."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def score(case, output):
    numbers = list(map(int, case.split()))
    w, d, n = numbers[:3]
    areas = numbers[3:]
    coords = list(map(int, output.split()))
    if len(coords) != 4*d*n or len(areas) != d*n:
        raise ValueError("wrong number of rectangles")
    cost = 1
    old = None
    for day in range(d):
        h, v = {}, {}
        boxes = []
        for j in range(n):
            i = n*day+j
            a,b,c,e = coords[4*i:4*i+4]
            if not (0 <= a < c <= w and 0 <= b < e <= w):
                raise ValueError("rectangle outside hall or empty")
            for x,y,z,t in boxes:
                if max(a,x) < min(c,z) and max(b,y) < min(e,t):
                    raise ValueError("interior overlap")
            boxes.append((a,b,c,e))
            cost += 100*max(0, areas[i]-(c-a)*(e-b))
            for x in (a,c):
                if 0 < x < w:
                    h[x] = h.get(x,0) | ((1 << (e-b))-1) << b
            for y in (b,e):
                if 0 < y < w:
                    v[y] = v.get(y,0) | ((1 << (c-a))-1) << a
        if old is not None:
            cost += sum((a.get(i,0)^b.get(i,0)).bit_count()
                        for a,b in zip(old,(h,v)) for i in a.keys() | b.keys())
        old = (h,v)
    return cost


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
    solvers = {"baseline":args.baseline.resolve(),"strips":args.candidate.resolve()}
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
                      "optima":sum(r["score"]==1 for r in rows),
                      "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"]=sum(results[c.name,"strips"]["score"]<results[c.name,"baseline"]["score"]
                                  for c in files)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
