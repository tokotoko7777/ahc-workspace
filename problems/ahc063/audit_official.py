#!/usr/bin/env python3
"""Evaluate a frozen outsider under both official archived and shared rank populations."""
import argparse
from array import array
from bisect import bisect_left, bisect_right
import csv
import json
from pathlib import Path
import re
import subprocess
import time

from benchmark import replay, sha

N=2000
P=1264
SCALE=10**9
FIRST=("rhoo",1985255142455)
HASHES={
    "seeds":"daf8cb67f5deb3bcb35e3d34b97a3b7b7a3ee8da1fe1cbe24f39389ad64a6c85",
    "metadata":"6ffc3a537fb5f139e62ae469552f61556a24136ebf7091827e213e20185b449c",
    "historical":"bf4978fc4e89bff2ce492008a7dbc4b42cf0ccb49b41688208dc7b31555f59c6",
}


def judge(case,vis,solver,scratch):
    data=case.read_bytes()
    start=time.perf_counter()
    result=subprocess.run([solver],input=data,capture_output=True,check=True,timeout=2)
    wall=time.perf_counter()-start
    if wall>=2:
        raise TimeoutError("case over 2s")
    summary=replay(data,result.stdout)
    own=re.fullmatch(rb"Cost = (\d+)\s*",result.stderr)
    scratch.write_bytes(result.stdout)
    check=subprocess.run([vis,case,scratch],cwd=scratch.parent,capture_output=True,check=True,timeout=10)
    official=re.search(rb"Score = (\d+)",check.stdout+check.stderr)
    if not own or not official or int(own[1])!=summary["score"] or int(official[1])!=summary["score"]:
        raise ValueError("official, independent and internal score mismatch")
    return {**summary,"wall":wall}


def points(rank2,n):
    divisor=2*n
    return (2*SCALE*(divisor-rank2)+divisor)//(2*divisor)


def add_column(column,total,n):
    lo=0
    while lo<len(column):
        hi=lo+1
        while hi<len(column) and column[hi][0]==column[lo][0]:
            hi+=1
        value=points(lo+hi-1,n)
        for _,who in column[lo:hi]:
            total[who]+=value
        lo=hi


def describe(labels,scores,ours):
    hist=list(zip(labels,scores))
    return {"best_historical":max(hist,key=lambda a:a[1]),"self":ours,
            "self_rank":1+sum(x>ours for x in scores),"self_ties":sum(x==ours for x in scores)}


def main():
    p=argparse.ArgumentParser()
    for k in ("seeds","metadata","historical","cases","vis","solver","out"):
        p.add_argument(k,type=lambda x:Path(x).resolve())
    a=p.parse_args()
    for name,value in HASHES.items():
        if sha(getattr(a,name))!=value:
            raise ValueError(f"official {name} hash mismatch")
    seeds=[int(v) for v in a.seeds.read_text().split()]
    with a.metadata.open() as f:
        reader=csv.DictReader(f)
        assert reader.fieldnames==["file","seed","N","M","C"]
        rows=list(reader)
    if len(seeds)!=N or seeds!=[int(r["seed"]) for r in rows]:
        raise ValueError("seed order differs")
    cases=sorted(a.cases.glob("*.txt"))
    if [c.stem for c in cases]!=[r["file"] for r in rows] or len(cases)!=N:
        raise ValueError("input filenames differ")
    for case,r in zip(cases,rows):
        with case.open() as fh:
            actual=list(map(int,fh.readline().split()))
        if actual!=[int(r[key]) for key in ("N","M","C")]:
            raise ValueError("input dimensions differ")
    labels=[]; costs=[]
    with a.historical.open() as f:
        rd=csv.reader(f)
        if next(rd)[:2]!=["rank_min",str(SCALE)]:
            raise ValueError("wrong rank archive formula")
        for row in rd:
            labels.append(row[0])
            costs.append(array("i",map(int,row[1:])))
    if len(labels)!=P or any(len(row)!=N for row in costs):
        raise ValueError("wrong archive shape")
    fixed=[0]*P
    for raw in zip(*costs):
        valid=sorted((v,i) for i,v in enumerate(raw) if v>=0)
        add_column(valid,fixed,P)
    if max(zip(labels,fixed),key=lambda x:x[1])!=FIRST:
        raise ValueError("archived winner does not agree")

    a.out.mkdir(parents=True,exist_ok=True)
    manifest={"official":HASHES,"runner":sha(Path(__file__)),"replay":sha(Path(__file__).with_name("benchmark.py")),
              "vis":sha(a.vis),"solver":sha(a.solver),"cases":{c.name:sha(c) for c in cases}}
    doc=a.out/"manifest.json"
    if doc.exists() and json.loads(doc.read_text())!=manifest:
        raise ValueError("stale result")
    doc.write_text(json.dumps(manifest,indent=2)+"\n")
    print(f"{N}/{N} seed, dimensions, and historical winner matched before testing",flush=True)
    checks=a.out/"checkpoints"
    checks.mkdir(exist_ok=True)
    ours=[]; perfect=0; max_wall=0.
    for i,case in enumerate(cases,1):
        f=checks/f"{case.stem}.json"
        if f.exists():
            r=json.loads(f.read_text())
        else:
            r=judge(case,a.vis,a.solver,a.out/"solution.txt")
            tmp=f.with_suffix(".tmp")
            tmp.write_text(json.dumps(r)+"\n")
            tmp.replace(f)
        ours.append(r["score"])
        perfect+=r["perfect"]
        max_wall=max(max_wall,r["wall"])
        if i%250==0:
            print(f"{i}/{N} legal and exact",flush=True)
    (a.out/"solution.txt").unlink(missing_ok=True)
    outsider=0; combined=[0]*(P+1)
    for raw,cost in zip(zip(*costs),ours):
        valid=sorted((v,i) for i,v in enumerate(raw) if v>=0)
        # Outsider has no historical self tie to exclude. Historical comparisons
        # above exclude each row's own occurrence through lo+hi-1.
        lo=bisect_left(valid,(cost,-1))
        hi=bisect_right(valid,(cost,P))
        outsider+=points(lo+hi,P)
        valid.insert(hi,(cost,P))
        add_column(valid,combined,P+1)
    summary={"legal":len(ours),"raw":sum(ours),"perfect":perfect,"max_wall":max_wall,
             "historical_cdf":describe(labels,fixed,outsider),
             "hypothetical_common_population":describe(labels,combined[:-1],combined[-1])}
    (a.out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary),flush=True)


if __name__=="__main__":
    main()
