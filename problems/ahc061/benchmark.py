#!/usr/bin/env python3
"""Use official interactive tester and independently replay all joint moves."""
import argparse
from collections import deque
import hashlib
import json
import math
import os
from pathlib import Path
import re
import subprocess
import time


def replay(data, turns):
    words=data.split()
    n,m,t,u=map(int,words[:4])
    fields=list(map(int,words[4:4+n*n+2*m]))
    values=fields[:n*n]
    starts=fields[n*n:n*n+2*m]
    pos=[starts[2*p]*n+starts[2*p+1] for p in range(m)]
    adj=[tuple((i+a)*n+j+b for a,b in ((0,1),(1,0),(-1,0),(0,-1))
               if 0<=i+a<n and 0<=j+b<n) for i in range(n) for j in range(n)]
    owner=[-1]*(n*n)
    level=[0]*(n*n)
    for p, cell in enumerate(pos):
        owner[cell]=p
        level[cell]=1
    if len(turns)!=t:
        raise ValueError("wrong turn count")
    for actions in turns:
        if len(actions)!=m:
            raise ValueError("wrong move count")
        for p, target in enumerate(actions):
            if not 0<=target<n*n or target in pos[:p]+pos[p+1:]:
                raise ValueError("wrong target")
            seen={pos[p]}
            queue=deque(seen)
            while queue:
                x=queue.popleft()
                for y in adj[x]:
                    if y not in seen and owner[y]==p:
                        seen.add(y)
                        queue.append(y)
            if target not in seen and not any(x in seen for x in adj[target]):
                raise ValueError("unreachable target")
        players={}
        for p,a in enumerate(actions):
            players.setdefault(a,[]).append(p)
        nextpos=pos.copy()
        for a,contest in players.items():
            own=owner[a]
            if len(contest)>1:
                if own not in contest:
                    continue
                p=own
            else:
                p=contest[0]
            if own==p:
                level[a]=min(u,level[a]+1)
            elif level[a]<=1:
                owner[a]=p
                level[a]=1
            else:
                level[a]-=1
                continue
            nextpos[p]=a
        pos=nextpos
    land=[0]*m
    for i,p in enumerate(owner):
        if p>=0:
            land[p]+=values[i]*level[i]
    return math.floor(1e5*math.log2(1+land[0]/max(land[1:]))+.5)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def evaluate(case, tester, solver):
    data=case.read_bytes()
    start=time.perf_counter()
    result=subprocess.run([tester,solver],input=data,capture_output=True,check=True,timeout=2,
                          env={**os.environ,"AHC061_TRACE":"1"})
    elapsed=time.perf_counter()-start
    if elapsed>=2:
        raise TimeoutError(case)
    turns=[list(map(int,x.split())) for x in re.findall(rb"^Turn (.+)$",result.stderr,re.M)]
    independent=replay(data,turns)
    official=re.findall(rb"^Score = (\d+)$",result.stderr,re.M)
    own=re.findall(rb"^Raw score = (\d+)$",result.stderr,re.M)
    if len(official)!=1 or len(own)!=1 or int(official[0])!=independent or int(own[0])!=independent:
        raise ValueError((case,solver,independent,result.stderr[-1000:]))
    return {"score":independent,"wall":elapsed,
            "moves":hashlib.sha256(repr(turns).encode()).hexdigest()}


def main():
    p=argparse.ArgumentParser()
    for label in ("input_dir","output_dir","tester","control","candidate"):
        p.add_argument(label,type=lambda p: Path(p).resolve())
    a=p.parse_args()
    a.output_dir.mkdir(parents=True,exist_ok=True)
    files=sorted(a.input_dir.glob("*.txt"))
    assert files, "no inputs"
    solvers={"control":a.control,"candidate":a.candidate}
    manifest={"runner":sha(Path(__file__)),"tester":sha(a.tester),
              "binaries":{k:sha(v) for k,v in solvers.items()},"inputs":{p.name:sha(p) for p in files}}
    meta=a.output_dir/"manifest.json"
    if meta.exists() and json.loads(meta.read_text())!=manifest:
        raise ValueError("stale checkpoints")
    meta.write_text(json.dumps(manifest,indent=2)+"\n")
    rows={k:[] for k in solvers}
    for i,case in enumerate(files):
        for k in (solvers if i%2==0 else reversed(solvers)):
            path=a.output_dir/f"{case.stem}-{k}.json"
            if path.exists():
                r=json.loads(path.read_text())
            else:
                r=evaluate(case,a.tester,solvers[k])
                tmp=path.with_suffix(".tmp")
                tmp.write_text(json.dumps(r)+"\n")
                tmp.replace(path)
            rows[k].append(r)
    summary={k:{"legal":len(r),"score":sum(s["score"] for s in r),
                "max_wall":max(s["wall"] for s in r)} for k,r in rows.items()}
    scores=[y["score"]-x["score"] for x,y in zip(rows["control"],rows["candidate"])]
    summary.update(wins=sum(x>0 for x in scores),losses=sum(x<0 for x in scores),
                   ties=sum(x==0 for x in scores))
    (a.output_dir/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary),flush=True)


if __name__=="__main__":
    main()
