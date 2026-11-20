#!/usr/bin/env python3
"""Run the independent snake bite-and-food replay plus official vis."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def replay(case, output):
    n,m,c,*rest=map(int,case.split())
    wanted=rest[:m]
    food={(i//n,i%n):a for i,a in enumerate(rest[m:]) if a}
    body=[(4,0),(3,0),(2,0),(1,0),(0,0)]
    colors=[1]*5
    ops=output.decode().split()
    bites=0
    if len(ops)>100000:
        raise ValueError("turn limit")
    for action in ops:
        di,dj={"U":(-1,0),"D":(1,0),"L":(0,-1),"R":(0,1)}[action]
        x,y=body[0][0]+di,body[0][1]+dj
        cell=(x,y)
        if not (0<=x<n and 0<=y<n) or cell==body[1]:
            raise ValueError("illegal motion")
        if cell in food:
            body=[cell]+body
            colors.append(food.pop(cell))
        else:
            body=[cell]+body[:-1]
            if cell in body[1:-1]:
                cut=body.index(cell,1)+1
                for pos,value in zip(body[cut:],colors[cut:]):
                    if pos in food:
                        raise ValueError("overlapping food in replay")
                    food[pos]=value
                body=body[:cut]
                colors=colors[:cut]
                bites+=1
        if len(food)+len(colors)!=m:
            raise ValueError("food conservation failed")
    errors=sum(a!=b for a,b in zip(colors,wanted))
    score=len(ops)+10000*(errors+2*(m-len(colors)))
    return {"cost":score,"score":score,"bites":bites,"moves":len(ops),"perfect":score==len(ops)}


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
                  "bites":sum(s["bites"] for s in series),"perfect":sum(s["perfect"] for s in series)} for key,series in rows.items()}
    results["wins"]=sum(a["score"]>b["score"] for a,b in zip(rows["control"],rows["candidate"]))
    results["losses"]=sum(a["score"]<b["score"] for a,b in zip(rows["control"],rows["candidate"]))
    (a.output_dir/"summary.json").write_text(json.dumps(results,indent=2)+"\n")
    print(json.dumps(results),flush=True)


if __name__ == "__main__":
    main()
