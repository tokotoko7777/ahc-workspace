#!/usr/bin/env python3
"""Reproduce AHC025 historical-case rank metrics, with no solver tuning."""
from array import array
from collections import Counter
import argparse
import csv
import json
from pathlib import Path
import re
import subprocess
import time

from benchmark import sha, score

COEF = 10**8
COUNT = 5000
RANK_HASH = "3656f77a7b2c5c59e1e1e78d39eeb5c47d89f830e24d708da2beba914ec1dbeb"
INPUT_HASH = "f976c335d6768704d98d48ff174bab34fff41a4ab1b72d91d74fd142a07b91ad"
SEED_HASH = "8a39261299bef0387172c0e0c4523c49b0cb993efd4f702ec7cf5124cf5b4c55"


def points(size, better, other_tied):
    # Round positive rational exactly: rank counts each other tie half.
    return (COEF * (2*size-2*better-other_tied)+size)//(2*size)


def rank(history, candidate):
    names, rows = [], []
    with history.open(newline="") as f:
        reader = csv.reader(f)
        header = next(reader)
        if header[:2] != ["rank_min", str(COEF)]:
            raise ValueError("wrong historical score formula")
        for row in reader:
            user, *s = row
            if len(s) != COUNT:
                raise ValueError("wrong archived case count")
            names.append(user)
            rows.append(array('i', map(int,s)))
    if len(names) != 1085 or len(set(names)) != len(names):
        raise ValueError("wrong archived entrant count")
    fixed_totals = [0]*len(names)
    expanded_totals = fixed_totals.copy()
    fixed_self=expanded_self=0
    for ours, column in zip(candidate,zip(*rows),strict=True):
        counts = Counter(s for s in column if s>=0)
        better=0
        fixed,expanded={},{}
        for s in sorted(counts):
            same=counts[s]
            fixed[s]=points(len(names),better,same-1)
            expanded[s]=points(len(names)+1,better+int(ours<s),same-1+int(ours==s))
            better+=same
        better=sum(n for s,n in counts.items() if s<ours)
        fixed_self+=points(len(names),better,counts[ours])
        expanded_self+=points(len(names)+1,better,counts[ours])
        for k,s in enumerate(column):
            if s>=0:
                fixed_totals[k]+=fixed[s]
                expanded_totals[k]+=expanded[s]
    fixed=sorted(zip(fixed_totals,names),reverse=True)
    expanded=sorted(zip(expanded_totals,names),reverse=True)
    if fixed[0][0]!=492832027444:
        raise ValueError(f"historical #1 does not match official archive: {fixed[0]}")
    return {
        "historical": {"population":len(names),"top3":fixed[:3],"candidate":fixed_self,
                       "candidate_projected_rank":1+sum(s>fixed_self for s in fixed_totals),
                       "candidate_ties":sum(s==fixed_self for s in fixed_totals)},
        "added": {"population":len(names)+1,"archive_top3":expanded[:3],
                  "original_winner":expanded_totals[names.index(fixed[0][1])],
                  "candidate":expanded_self,
                  "candidate_rank":1+sum(s>expanded_self for s in expanded_totals),
                  "candidate_ties":sum(s==expanded_self for s in expanded_totals)}
    }


def main():
    p=argparse.ArgumentParser()
    for name in ("cases","output","solver","tester","generator","seeds","inputs","history"):
        p.add_argument(name,type=Path)
    p.add_argument("--source", required=True)
    a=p.parse_args()
    for file,want in [(a.seeds,SEED_HASH),(a.inputs,INPUT_HASH),(a.history,RANK_HASH)]:
        if sha(file)!=want:
            p.error(f"unexpected official file SHA-256 for {file}")
    seeds=a.seeds.read_text().splitlines()
    with a.inputs.open(newline="") as f:
        inputs=list(csv.DictReader(f))
    cases=[a.cases/f"{i:04}.txt" for i in range(COUNT)]
    if len(inputs)!=COUNT or len(seeds)!=COUNT or len(set(seeds))!=COUNT:
        p.error("incorrect official seed count")
    for i,(inp,seed,case) in enumerate(zip(inputs,seeds,cases,strict=True)):
        if inp["file"]!=f"{i:04}" or inp["seed"]!=seed:
            p.error("wrong official archive input ordering")
        if case.read_bytes().split()[:3]!=[inp[k].encode() for k in ("N","D","Q")]:
            p.error("generated features disagree with official archive")
    a.output.mkdir(parents=True,exist_ok=True)
    manifest=a.output/"manifest.json"
    data={"source":a.source,"programs":{
        str(v.name):sha(v) for v in (a.solver,a.tester,a.generator,Path(__file__),
                                    Path(__file__).with_name("benchmark.py"))},
          "official":{"seed":SEED_HASH,"input":INPUT_HASH,"history":RANK_HASH},
          "cases":{c.name:sha(c) for c in cases}}
    if manifest.exists() and json.loads(manifest.read_text())!=data:
        p.error("stale run")
    manifest.write_text(json.dumps(data,indent=2)+"\n")
    scores=[]
    worst=0.
    queries=0
    for i,case in enumerate(cases):
        checkpoint=a.output/f"{case.stem}.json"
        if checkpoint.exists():
            row=json.loads(checkpoint.read_text())
        else:
            start=time.monotonic()
            r=subprocess.run([str(a.tester.resolve()),str(a.solver.resolve())],
                             input=case.read_bytes(),capture_output=True,timeout=2,check=True)
            elapsed=time.monotonic()-start
            if elapsed>=2:
                raise TimeoutError("official tool+solver wall time exceeded 2s")
            official=re.fullmatch(rb"Score = (\d+)\s*",r.stderr)
            value,n,d,q=score(case,r.stdout.decode("ascii").strip())
            if not official or value!=int(official[1]):
                raise ValueError(f"official and independent disagree at case {i}")
            row={"absolute":value,"wall":elapsed,"q":q}
            tmp=checkpoint.with_suffix(".tmp")
            tmp.write_text(json.dumps(row)+"\n")
            tmp.replace(checkpoint)
        scores.append(row["absolute"])
        worst=max(worst,row["wall"])
        queries+=row["q"]
        if (i+1)%250==0:
            print(f"{i+1}/{COUNT} legal, official=independent",flush=True)
    summary={"valid":len(scores),"raw_total":sum(scores),"queries":queries,
             "max_tester_plus_solver_wall":worst,"archive":rank(a.history,scores)}
    (a.output/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary),flush=True)


if __name__=="__main__":
    main()
