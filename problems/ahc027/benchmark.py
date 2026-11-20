#!/usr/bin/env python3
"""Independently replay a legal closed route and compare its second-period dirt with official vis."""
import argparse
import hashlib
import json
import re
import subprocess
import time
from pathlib import Path


def score(case,answer):
    data=case.read_text().split()
    n=int(data[0])
    h=data[1:n]
    v=data[n:2*n]
    weights=list(map(int,data[2*n:]))
    if len(weights)!=n*n or re.fullmatch(r"[UDLR]{1,100000}",answer) is None:
        raise ValueError("bad route alphabet or length")
    r=c=0
    delta={"U":(-1,0),"D":(1,0),"L":(0,-1),"R":(0,1)}
    route=[]
    visited={0}
    for a in answer:
        dy,dx=delta[a]
        y,x=r+dy,c+dx
        if not (0<=y<n and 0<=x<n):
            raise ValueError("left the grid")
        if (h[min(r,y)][c] if dy else v[r][min(c,x)])!="0":
            raise ValueError("crossed a wall")
        r,c=y,x
        i=r*n+c
        visited.add(i);route.append(i)
    if (r,c)!=(0,0) or len(visited)!=n*n:
        raise ValueError("not a closed covering walk")
    length=len(route)
    last=[0]*len(weights)
    dirt=total=0
    new=sum(weights)
    # Simulate actual dirt recurrence over two periods, not the solver's
    # cyclic-gap formula.
    for t in range(1,2*length):
        i=route[(t-1)%length]
        dirt+=new-(t-last[i])*weights[i]
        last[i]=t
        if t>=length:
            total+=dirt
    return (2*total+length)//(2*length), length, n

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    for key in ("inputs", "out", "vis", "baseline", "candidate"):
        parser.add_argument(key, type=Path)
    parser.add_argument("--candidate-label",default="splice")
    a = parser.parse_args()
    cases = sorted(a.inputs.resolve().glob("*.txt"))
    out, vis = a.out.resolve(), a.vis.resolve()
    solvers = [("baseline", a.baseline.resolve()), (a.candidate_label, a.candidate.resolve())]
    out.mkdir(parents=True, exist_ok=True)
    scratch = out/"scratch"
    scratch.mkdir(exist_ok=True)
    manifest = out/"manifest.json"
    meta = {"runner":sha(Path(__file__)), "vis":sha(vis),
            "solvers":{name:sha(path) for name,path in solvers},
            "inputs":{p.name:sha(p) for p in cases}}
    if not cases or manifest.exists() and json.loads(manifest.read_text()) != meta:
        parser.error("no inputs or stale checkpoints")
    manifest.write_text(json.dumps(meta, indent=2)+"\n")
    results = {}
    for i, case in enumerate(cases):
        for name, binary in (solvers if i%2 == 0 else solvers[::-1]):
            p = out/f"{case.stem}-{name}.json"
            if p.exists():
                row = json.loads(p.read_text())
            else:
                start = time.perf_counter()
                run = subprocess.run([binary], input=case.read_bytes(), capture_output=True,
                                     check=True, timeout=2)
                wall = time.perf_counter() - start
                if wall > 2:
                    raise TimeoutError("execution exceeded 2 seconds")
                actions = run.stdout.decode("ascii").strip()
                s, length, n = score(case,actions)
                output = scratch/"output.txt"
                output.write_text(actions+"\n")
                actual = subprocess.run([vis, case, output], cwd=scratch, capture_output=True,
                                        text=True, check=True, timeout=10)
                match = re.fullmatch(r"Score = (\d+)\s*", actual.stdout)
                if match is None or int(match[1]) != s:
                    raise ValueError(f"official disagreement: {case} {s}, {actual.stdout!r}")
                row = {"score":s,"wall":wall,"n":n,"length":length}
                tmp = p.with_suffix(".tmp")
                tmp.write_text(json.dumps(row)+"\n")
                tmp.rename(p)
            results[name, case.name] = row
    summary = {}
    for name, _ in solvers:
        rows = [results[name,c.name] for c in cases]
        summary[name] = {"valid":len(rows), "total":sum(r["score"] for r in rows),
                          "route_length":sum(r["length"] for r in rows),
                          "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"] = sum(results[a.candidate_label,c.name]["score"] <
                                    results["baseline",c.name]["score"] for c in cases)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
