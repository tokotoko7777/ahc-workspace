#!/usr/bin/env python3
"""Replay every grid motion and card/deck mutation independently."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def replay(case, output):
    n, *grid = map(int, case.split())
    assert len(grid)==n*n
    ops = output.decode().split()
    if len(ops)>2*n**3:
        raise ValueError("operation limit")
    moves=placed=0
    i=j=0
    deck=[]
    for key in ops:
        at=i*n+j
        if key=="Z":
            if grid[at] < 0:
                raise ValueError("pick from empty cell")
            number=grid[at]
            grid[at]=-1
            if deck and deck[-1]==number:
                deck.pop()
            else:
                deck.append(number)
        elif key=="X":
            if grid[at]>=0 or not deck:
                raise ValueError("illegal drop")
            grid[at]=deck.pop()
            placed+=1
        else:
            di,dj={"U":(-1,0),"D":(1,0),"L":(0,-1),"R":(0,1)}[key]
            i,j=i+di,j+dj
            if not (0<=i<n and 0<=j<n):
                raise ValueError("outside board")
            moves+=1
    remaining=len(deck)+sum(x>=0 for x in grid)
    score=n*n+2*n**3-moves if remaining==0 else n*n-remaining
    return {"cost":moves,"score":score,"remaining":remaining,"placed":placed}


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
            internal=re.fullmatch(rb"Moves = (\d+)\s*",result.stderr)
            scratch.write_bytes(result.stdout)
            vis=subprocess.run([a.vis,case,scratch],cwd=a.output_dir,capture_output=True,check=True,timeout=10)
            official=re.search(rb"Score = (\d+)",vis.stdout+vis.stderr)
            if (summary["remaining"] or not official or not internal or int(official[1]) != summary["score"]
                or int(internal[1]) != summary["cost"]):
                raise ValueError((case,key,summary,result.stderr,vis.stdout,vis.stderr))
            summary["wall"]=wall
            tmp=row.with_suffix(".tmp")
            tmp.write_text(json.dumps(summary)+"\n")
            tmp.replace(row)
            rows[key].append(summary)
    results={key:{"valid":len(series),"score":sum(s["score"] for s in series),
                  "cost":sum(s["cost"] for s in series),"max_wall":max(s["wall"] for s in series),
                  "placed":sum(s["placed"] for s in series)} for key,series in rows.items()}
    results["wins"]=sum(a["score"]<b["score"] for a,b in zip(rows["control"],rows["candidate"]))
    results["losses"]=sum(a["score"]>b["score"] for a,b in zip(rows["control"],rows["candidate"]))
    (a.output_dir/"summary.json").write_text(json.dumps(results,indent=2)+"\n")
    print(json.dumps(results),flush=True)


if __name__ == "__main__":
    main()
