#!/usr/bin/env python3
"""Check objective and full simulation with independent per-point stepping."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import subprocess
import time


def replay(case, output):
    n, steps, m, k, period, *points = map(int, case.split())
    if len(points) != 4*n:
        raise ValueError("bad input")
    x, y, vx, vy = (list(points[i::4]) for i in range(4))
    actions = sorted(tuple(map(int, line.split())) for line in output.splitlines())
    if len(actions) != n-m:
        raise ValueError("bad edge count")
    by_time = [[] for _ in range(steps)]
    for t, i, j in actions:
        if not (0 <= t < steps and 0 <= i < n and 0 <= j < n):
            raise ValueError("bad operation")
        by_time[t].append((i,j))
    color = list(range(n))
    size = [1]*n
    total = 0
    for t in range(steps):
        for i, j in by_time[t]:
            a, b = color[i], color[j]
            if a == b or size[a]+size[b] > k:
                raise ValueError("cyclic or oversized component")
            dx, dy = abs(x[i]-x[j]), abs(y[i]-y[j])
            total += math.floor(math.hypot(min(dx,period-dx),min(dy,period-dy))+.5)
            sa, sb = size[a], size[b]
            ax, ay = (sa*vx[i]+sb*vx[j])/(sa+sb), (sa*vy[i]+sb*vy[j])/(sa+sb)
            for u in range(n):
                if color[u] in (a,b):
                    color[u] = a
                    vx[u], vy[u] = ax, ay
            size[a] += sb
            size[b] = 0
        for i in range(n):
            x[i],y[i] = (x[i]+vx[i]) % period, (y[i]+vy[i]) % period
    if [s for s in size if s] != [k]*m:
        raise ValueError("wrong final sizes")
    return {"cost":total,"score":math.floor(1e6*math.log2(period*(n-m)/(total+1))+.5),
            "moving":sum(t != actions[0][0] for t,_,_ in actions)}


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser()
    for label in ("input_dir","output_dir","vis","control","candidate"):
        p.add_argument(label,type=lambda p: Path(p).resolve())
    a=p.parse_args()
    a.output_dir.mkdir(parents=True,exist_ok=True)
    files=sorted(a.input_dir.glob("*.txt"))
    assert files, "no inputs"
    solvers={"control":a.control,"candidate":a.candidate}
    manifest={"runner":sha(Path(__file__)),"vis":sha(a.vis),"binaries":{key:sha(x) for key,x in solvers.items()},
              "inputs":{p.name:sha(p) for p in files}}
    meta=a.output_dir/"manifest.json"
    if meta.exists() and json.loads(meta.read_text()) != manifest:
        raise ValueError("stale checkpoints")
    meta.write_text(json.dumps(manifest,indent=2)+"\n")
    rows={key:[] for key in solvers}
    scratch=a.output_dir/"solution.txt"
    for idx, case in enumerate(files):
        for key in (solvers if idx%2==0 else reversed(solvers)):
            row=a.output_dir/f"{case.stem}-{key}.json"
            if row.exists():
                rows[key].append(json.loads(row.read_text()))
                continue
            data=case.read_bytes()
            start=time.perf_counter()
            result=subprocess.run([solvers[key]],input=data,capture_output=True,check=True,timeout=2)
            wall=time.perf_counter()-start
            if wall>=2:
                raise TimeoutError(wall)
            summary=replay(data,result.stdout)
            internal=re.fullmatch(rb"Cost = (\d+)\s*",result.stderr)
            scratch.write_bytes(result.stdout)
            vis=subprocess.run([a.vis,case,scratch],cwd=a.output_dir,capture_output=True,check=True,timeout=10)
            official=re.search(rb"Score = (\d+)",vis.stdout+vis.stderr)
            if (not official or not internal or int(official[1]) != summary["score"]
                or int(internal[1]) != summary["cost"]):
                raise ValueError((case,key,summary,result.stderr,vis.stdout,vis.stderr))
            summary["wall"]=wall
            tmp=row.with_suffix(".tmp")
            tmp.write_text(json.dumps(summary)+"\n")
            tmp.replace(row)
            rows[key].append(summary)
    results={key:{"valid":len(series),"score":sum(s["score"] for s in series),
                  "cost":sum(s["cost"] for s in series),"max_wall":max(s["wall"] for s in series),
                  "moving":sum(s["moving"]>0 for s in series)} for key,series in rows.items()}
    results["wins"]=sum(a["score"]<b["score"] for a,b in zip(rows["control"],rows["candidate"]))
    results["losses"]=sum(a["score"]>b["score"] for a,b in zip(rows["control"],rows["candidate"]))
    (a.output_dir/"summary.json").write_text(json.dumps(results,indent=2)+"\n")
    print(json.dumps(results),flush=True)


if __name__ == "__main__":
    main()
