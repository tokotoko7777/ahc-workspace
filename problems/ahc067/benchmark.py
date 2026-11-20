#!/usr/bin/env python3
"""Validate door and switch geometry and independently BFS the full game graph."""
import argparse
from collections import deque
import hashlib
import json
import math
from pathlib import Path
import re
import subprocess
import time


def replay(case, output):
    lines=case.splitlines()
    n,limit,k=map(int,lines[0].split())
    board=b"".join(lines[1:]).decode()
    assert len(board)==n*n
    data=iter(map(int,output.split()))
    doors={}
    nd=next(data)
    if not 0<=nd<=limit:
        raise ValueError("invalid number of doors")
    for _ in range(nd):
        d,x,y,g=next(data),next(data),next(data),next(data)
        if d not in (0,1) or not (0<=x<n and 0<=y<n and 0<=g<2*k):
            raise ValueError("invalid door placement")
        xx,yy=x+(d==0),y+(d==1)
        if xx>=n or yy>=n:
            raise ValueError("door outside grid")
        edge=(x*n+y,xx*n+yy)
        if edge in doors:
            raise ValueError("duplicate door")
        doors[edge]=g
    switches={}
    ns=next(data)
    if not 0<=ns<=n*n:
        raise ValueError("bad switch count")
    for _ in range(ns):
        x,y,s=next(data),next(data),next(data)
        place=x*n+y
        if not (0<=x<n and 0<=y<n and 0<=s<k) or place in switches:
            raise ValueError("invalid or duplicate switch")
        switches[place]=s
    if next(data,None) is not None:
        raise ValueError("extra output")
    adj=[[] for _ in board]
    for x in range(n):
        for y in range(n):
            a=x*n+y
            if board[a]=='#':
                continue
            for xx,yy in ((x+1,y),(x,y+1)):
                b=xx*n+yy
                if xx<n and yy<n and board[b]!='#':
                    edge=doors.get((a,b),-1)
                    adj[a].append((b,edge))
                    adj[b].append((a,edge))
    dq=deque([(0,0,0)])
    seen={(0,0)}
    dest=n*n-1
    answer=None
    while dq:
        step,a,mask=dq.popleft()
        if a==dest:
            answer=step
            break
        for b,g in adj[a]:
            if (g<0 or (mask>>(g//2))%2==g%2) and (b,mask) not in seen:
                seen.add((b,mask))
                dq.append((step+1,b,mask))
        if a in switches:
            changed=mask^(1<<switches[a])
            if (a,changed) not in seen:
                seen.add((a,changed))
                dq.append((step+1,a,changed))
    if answer is None:
        raise ValueError("expected a reachable throne")
    return {"cost":answer,"score":math.floor(1e6*math.log2(answer/n)+.5),"switches":ns}


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
            internal=re.fullmatch(rb"Steps = (\d+)\s*",result.stderr)
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
                  "with_switches":sum(s["switches"]>0 for s in series)} for key,series in rows.items()}
    results["wins"]=sum(a["score"]<b["score"] for a,b in zip(rows["control"],rows["candidate"]))
    results["losses"]=sum(a["score"]>b["score"] for a,b in zip(rows["control"],rows["candidate"]))
    (a.output_dir/"summary.json").write_text(json.dumps(results,indent=2)+"\n")
    print(json.dumps(results),flush=True)


if __name__ == "__main__":
    main()
