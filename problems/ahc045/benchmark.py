#!/usr/bin/env python3
"""Official lock-step queries plus independent transcript and exact length check."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import subprocess
import time


def score(case, transcript):
    n,m,limit,maximum,width,*data=map(int,case.split())
    counts=data[:m]
    xy=list(zip(data[m+4*n::2],data[m+4*n+1::2],strict=True))
    if len(xy)!=n:
        raise ValueError("bad generated instance")
    lines=iter(transcript.splitlines())
    query=[]
    for text in lines:
        if text==b"!":
            break
        tag,qty,*rest=text.split()
        qty=int(qty)
        cities=tuple(map(int,rest))
        if (tag!=b"?" or len(cities)!=qty or not 2<=qty<=maximum or
                len(set(cities))!=qty or any(not 0<=v<n for v in cities)):
            raise ValueError("invalid question")
        query.append(cities)
    else:
        raise ValueError("missing final-answer marker")
    if len(query)>limit:
        raise ValueError("too many queries")
    group=[]
    assigned=set()
    total=0
    for count in counts:
        cities=tuple(map(int,next(lines).split()))
        members=set(cities)
        if (len(cities)!=count or len(members)!=count or
                assigned & members or any(not 0<=v<n for v in members)):
            raise ValueError("invalid city group")
        group.append(cities)
        assigned.update(members)
        parent={v:v for v in cities}
        def find(x):
            while x!=parent[x]:
                x=parent[x]
            return x
        for _ in range(count-1):
            a,b=map(int,next(lines).split())
            if a not in parent or b not in parent or find(a)==find(b):
                raise ValueError("invalid tree road or cycle")
            parent[find(a)]=find(b)
            x,y=xy[a]; r,c=xy[b]
            total+=math.isqrt((x-r)**2+(y-c)**2)
    if next(lines,None) is not None or len(assigned)!=n:
        raise ValueError("trailing output or missing city")
    return {"score":total,"queries":len(query),
            "decision_hash":hashlib.sha256(repr((query,group)).encode()).hexdigest()}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser()
    for name in ("inputs","out","tester","baseline","candidate"):
        parser.add_argument(name,type=lambda x:Path(x).resolve())
    a=parser.parse_args()
    cases=sorted(a.inputs.glob("*.txt"))
    a.out.mkdir(parents=True,exist_ok=True)
    solvers={"baseline":a.baseline,"ordinal":a.candidate}
    file=a.out/"manifest.json"
    metadata={"checker":sha(Path(__file__)),"tester":sha(a.tester),
              "solvers":{k:sha(p) for k,p in solvers.items()},
              "inputs":{p.name:sha(p) for p in cases}}
    if not cases or file.exists() and json.loads(file.read_text())!=metadata:
        parser.error("absent inputs or stale checkpoints")
    file.write_text(json.dumps(metadata,indent=2)+"\n")
    results={}
    for i,case in enumerate(cases):
        for name in (solvers if i%2==0 else reversed(solvers)):
            path=a.out/f"{case.stem}-{name}.json"
            if path.exists():
                row=json.loads(path.read_text())
            else:
                data=case.read_bytes()
                started=time.monotonic()
                run=subprocess.run([a.tester,solvers[name]],input=data,capture_output=True,
                                   check=True,timeout=2)
                wall=time.monotonic()-started
                expected=re.fullmatch(rb"Score = (\d+)\s*",run.stderr)
                row=score(data,run.stdout)
                if wall>=2 or not expected or row["score"]!=int(expected[1]):
                    raise ValueError(f"invalid tester result for {case}: {run.stderr!r}, {row}")
                row["wall"]=wall
                tmp=path.with_suffix(".tmp")
                tmp.write_text(json.dumps(row)+"\n")
                tmp.replace(path)
            results[case.name,name]=row
        if results[case.name,"baseline"]["decision_hash"]!=results[case.name,"ordinal"]["decision_hash"]:
            raise ValueError("changed queries/grouping; not a matched comparison")
    summary={}
    for name in solvers:
        rows=[results[c.name,name] for c in cases]
        summary[name]={"valid":len(rows),"total":sum(r["score"] for r in rows),
                       "queries":sum(r["queries"] for r in rows),"max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"]=sum(results[c.name,"ordinal"]["score"]<results[c.name,"baseline"]["score"]
                                 for c in cases)
    (a.out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__=="__main__":
    main()
