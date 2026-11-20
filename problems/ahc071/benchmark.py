#!/usr/bin/env python3
"""Validate all brick cells and supports by a simple independent rasterizer."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def replay(case, output):
    w,h,k,*data=map(int,case.split())
    price=dict(zip((1,3,5,7,9),data[:5]))
    holes=list(zip(data[5::2],data[6::2]))
    if len(holes)!=k: raise ValueError("invalid holes")
    count,*bricks=map(int,output.split())
    if not 0<=count<=w*h or len(bricks)!=3*count:
        raise ValueError("invalid brick count")
    board=[[False]*w for _ in range(h)]
    supports=[]
    cost=0
    for x,y,l in zip(bricks[::3],bricks[1::3],bricks[2::3]):
        if l not in price or not (0<=x and x+l<=w and 0<=y<h):
            raise ValueError("illegal brick")
        for col in range(x,x+l):
            if board[y][col]: raise ValueError("overlap")
            board[y][col]=True
        if y: supports.append((x+(l-1)//2,y-1))
        cost+=price[l]
    if any(not board[y][x] for x,y in supports): raise ValueError("unsupported center")
    if any(not board[y][x] for x,y in holes): raise ValueError("uncovered hole")
    return {"cost":cost,"score":max(0,w*h*price[1]-cost+1),"bricks":count}


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
                  "bricks":sum(s["bricks"] for s in series)} for key,series in rows.items()}
    results["wins"]=sum(a["score"]<b["score"] for a,b in zip(rows["control"],rows["candidate"]))
    results["losses"]=sum(a["score"]>b["score"] for a,b in zip(rows["control"],rows["candidate"]))
    (a.output_dir/"summary.json").write_text(json.dumps(results,indent=2)+"\n")
    print(json.dumps(results),flush=True)


if __name__ == "__main__":
    main()
