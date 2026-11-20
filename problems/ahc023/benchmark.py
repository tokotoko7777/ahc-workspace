#!/usr/bin/env python3
"""Independent day-by-day accessibility replay plus official vis check."""
import argparse
import hashlib
import json
import re
import subprocess
import time
from pathlib import Path


def score(case,answer):
    lines=case.read_text().splitlines()
    t,h,w,entrance=map(int,lines[0].split())
    horizontal=lines[1:h]
    vertical=lines[h:2*h]
    k=int(lines[2*h])
    crops=[tuple(map(int,q.split())) for q in lines[2*h+1:]]
    assert len(crops)==k
    n=h*w
    adj=[[] for _ in range(n)]
    def join(a,b):
        adj[a].append(b)
        adj[b].append(a)
    for r,row in enumerate(horizontal):
        for c,a in enumerate(row):
            if a=='0':
                join(r*w+c,(r+1)*w+c)
    for r,row in enumerate(vertical):
        for c,a in enumerate(row):
            if a=='0':
                join(r*w+c,r*w+c+1)
    count,*raw=map(int,answer.split())
    if len(raw)!=count*4 or not 0<=count<=k:
        raise ValueError("wrong plan size")
    planting=[[] for _ in range(t+1)]
    harvest=[[] for _ in range(t+1)]
    used=set()
    value=0
    for crop,r,c,start in zip(raw[0::4],raw[1::4],raw[2::4],raw[3::4],strict=True):
        if not 1<=crop<=k or crop in used or not (0<=r<h and 0<=c<w):
            raise ValueError("duplicate crop or bad coordinates")
        latest,end=crops[crop-1]
        if not 1<=start<=latest:
            raise ValueError("invalid planting schedule")
        used.add(crop)
        cell=r*w+c
        planting[start].append((crop,cell))
        harvest[end].append((crop,cell))
        value+=end-latest+1
    root=entrance*w
    ongoing=[0]*n
    def reach(harvestable):
        if ongoing[root] and ongoing[root] not in harvestable:
            return set()
        seen={root}
        todo=[root]
        for v in todo:
            for a in adj[v]:
                if a not in seen and (not ongoing[a] or ongoing[a] in harvestable):
                    seen.add(a)
                    todo.append(a)
        return seen
    for day in range(1,t+1):
        # All accessible planting targets can be visited deepest first in a
        # BFS tree of currently empty cells. Harvest by the reverse logic.
        access=reach(())
        for crop,cell in planting[day]:
            if cell not in access or ongoing[cell]:
                raise ValueError("inaccessible or occupied planting cell")
            ongoing[cell]=crop
        access=reach({crop for crop,_ in harvest[day]})
        for crop,cell in harvest[day]:
            if cell not in access or ongoing[cell]!=crop:
                raise ValueError("inaccessible harvest")
            ongoing[cell]=0
    if any(ongoing):
        raise ValueError("unharvested crop")
    score,rem=divmod(1000000*value,h*w*t)
    if rem:
        raise ValueError("non-integral score on official-size input")
    return score,count

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    for key in ("inputs", "out", "vis", "baseline", "two_ended"):
        parser.add_argument(key, type=Path)
    parser.add_argument("--candidate-label",default="windows")
    a = parser.parse_args()
    cases = sorted(a.inputs.resolve().glob("*.txt"))
    out, vis = a.out.resolve(), a.vis.resolve()
    solvers = [("baseline", a.baseline.resolve()), (a.candidate_label, a.two_ended.resolve())]
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
                output = scratch/"output.txt"
                output.write_text(actions+"\n")
                s, crops = score(case,actions)
                actual = subprocess.run([vis, case, output], cwd=scratch, capture_output=True,
                                        text=True, check=True, timeout=90)
                match = re.fullmatch(r"Score = (\d+)\s*", actual.stdout)
                if match is None or int(match[1]) != s:
                    raise ValueError(f"official disagreement: {case} {s}, {actual.stdout!r}")
                row = {"score":s, "wall":wall, "crops":crops}
                tmp = p.with_suffix(".tmp")
                tmp.write_text(json.dumps(row)+"\n")
                tmp.rename(p)
            results[name, case.name] = row
    summary = {}
    for name, _ in solvers:
        rows = [results[name,c.name] for c in cases]
        summary[name] = {"valid":len(rows), "total":sum(r["score"] for r in rows),
                          "crops":sum(r["crops"] for r in rows),
                          "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"] = sum(results[a.candidate_label,c.name]["score"] >
                                    results["baseline",c.name]["score"] for c in cases)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
