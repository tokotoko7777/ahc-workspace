#!/usr/bin/env python3
"""Check sheet visibility, post-removal depths, display and official order count."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def replay(data, answer):
    words=data.split()
    n,k,p,r=map(int,words[:4])
    catalogue=[int(x,2) for x in words[4:4+p]]
    orders=list(map(int,words[4+p:]))
    if len(orders)!=r: raise ValueError("wrong input order count")
    out=iter(answer.split())
    sheets=[]
    for _ in range(k):
        s=next(out)
        if len(s)!=n or set(s)-set(b"01."): raise ValueError("invalid sheet")
        sheets.append((int(s.replace(b"0",b"1").replace(b".",b"0"),2),
                       int(s.replace(b".",b"0"),2)))
    order=list(range(k))
    turns=errors=0
    full=(1<<n)-1
    for i in orders:
        count=int(next(out))
        if not 0<=count<=2*k: raise ValueError("operations per order exceeded")
        turns+=count
        for _ in range(count):
            sheet,d=int(next(out)),int(next(out))
            if not (0<=sheet<k and 0<=d<k): raise ValueError("move or depth out of bounds")
            covered=0
            target=order.index(sheet)
            for j in range(target):
                covered|=sheets[order[j]][0]
                if covered==full: break
            if sheets[sheet][0]&~covered==0: raise ValueError("sheet is not visible")
            order.insert(d,order.pop(target))
        color=covered=0
        for idx in order:
            ink,actual=sheets[idx]
            color|=actual&~covered
            covered|=ink
            if covered==full: break
        errors+=color!=catalogue[i]
    if next(out,None) is not None: raise ValueError("extra output")
    return {"score":moves_cost(turns,errors,k),"turns":turns,"errors":errors}


def moves_cost(turns,errors,k):
    return turns+2*k*errors

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def evaluate(case, tester, solver):
    data=case.read_bytes()
    start=time.perf_counter()
    result=subprocess.run([tester,solver],input=data,capture_output=True,check=True,timeout=2)
    elapsed=time.perf_counter()-start
    if elapsed>=2: raise TimeoutError(case)
    independent=replay(data,result.stdout)
    official=re.findall(rb"^Score = (\d+)$",result.stderr,re.M)
    own=re.findall(rb"^Raw score = (\d+)$",result.stderr,re.M)
    if (len(official)!=1 or len(own)!=1 or int(official[0])!=independent["score"] or
        int(own[0])!=independent["score"] or independent["errors"]):
        raise ValueError((case,solver,independent,result.stderr[-1000:]))
    return {**independent,"wall":elapsed,"protocol":hashlib.sha256(result.stdout).hexdigest()}


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
                "max_wall":max(s["wall"] for s in r),
                "turns":sum(s["turns"] for s in r),"errors":sum(s["errors"] for s in r)} for k,r in rows.items()}
    scores=[x["score"]-y["score"] for x,y in zip(rows["control"],rows["candidate"])]
    summary.update(wins=sum(x>0 for x in scores),losses=sum(x<0 for x in scores),
                   ties=sum(x==0 for x in scores))
    (a.output_dir/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary),flush=True)


if __name__=="__main__":
    main()
