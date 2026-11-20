#!/usr/bin/env python3
"""Replay the tree, cumulative rotations, root and fingertip actions."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def score(case, output):
    n, m, allowance, *blocks = case.split()
    n,m,allowance=map(int,(n,m,allowance))
    board={(r,c) for r,row in enumerate(blocks[:n])
           for c,char in enumerate(row) if char==49}
    target={(r,c) for r,row in enumerate(blocks[n:])
            for c,char in enumerate(row) if char==49}
    lines=output.decode("ascii").splitlines()
    v=int(lines[0])
    if not 1<=v<=allowance or len(lines)<v+1:
        raise ValueError("invalid tree size")
    pairs=[tuple(map(int,line.split())) for line in lines[1:v]]
    parents=[-1]+[p for p,_ in pairs]
    lengths=[0]+[x for _,x in pairs]
    leaves=set(range(v))
    for u,(p,x) in enumerate(pairs,1):
        if not (0<=p<u and 1<=x<n):
            raise ValueError("invalid arm link")
        leaves.discard(p)
    r,c=map(int,lines[v].split())
    inside=lambda x,y:0<=x<n and 0<=y<n
    if not inside(r,c):
        raise ValueError("invalid initial root")
    relative=[0]*v
    held=set()
    dr,dc=(0,1,0,-1),(1,0,-1,0)
    movement={".":(0,0),"U":(-1,0),"D":(1,0),"L":(0,-1),"R":(0,1)}
    rotation={".":0,"L":3,"R":1}
    turns=len(lines)-v-1
    if turns>100000:
        raise ValueError("too many turns")
    for action in lines[v+1:]:
        if len(action)!=2*v or action[0] not in movement:
            raise ValueError("invalid movement record")
        x,y=movement[action[0]]
        r+=x
        c+=y
        if not inside(r,c):
            raise ValueError("root left board")
        positions=[(r,c)]
        absori=[0]*v
        for u in range(1,v):
            if action[u] not in rotation:
                raise ValueError("invalid rotation")
            relative[u]=(relative[u]+rotation[action[u]])%4
            p=parents[u]
            absori[u]=(relative[u]+absori[p])%4
            x,y=positions[p]
            positions.append((x+dr[absori[u]]*lengths[u],y+dc[absori[u]]*lengths[u]))
        for u,s in enumerate(action[v:]):
            if s==".":
                continue
            if s!="P" or u not in leaves or not inside(*positions[u]):
                raise ValueError("invalid fingertip action")
            here=positions[u]
            if u in held:
                if here in board:
                    raise ValueError("release onto occupied position")
                board.add(here)
                held.remove(u)
            else:
                if here not in board:
                    raise ValueError("pick at an empty position")
                board.remove(here)
                held.add(u)
    missing=len(target-board)
    if missing==0 and (board!=target or held):
        raise ValueError("unexpected extra pieces")
    return {"score":turns if missing==0 else 100000+1000*missing,"missing":missing}

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
    solvers = {"baseline":args.baseline.resolve(),"stations":args.candidate.resolve()}
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
                if (not official_value or actual["score"]!=int(official_value[1]) or
                        (response.stderr and (not internal or actual["score"]!=int(internal[1])))):
                    raise ValueError("official or solver disagreement")
                if actual["missing"]:
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
                      "missing":sum(r["missing"] for r in rows),
                      "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"]=sum(results[c.name,"stations"]["score"]<results[c.name,"baseline"]["score"]
                                  for c in files)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
