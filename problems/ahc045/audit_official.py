#!/usr/bin/env python3
"""Audit one frozen submission against AtCoder's published system cases."""
from array import array
import argparse
import csv
import json
from pathlib import Path
import re
import subprocess
import time

from benchmark import sha, score

COUNT=3000
COEF=10**9
HASHES={
    "seeds":"e4cb8e03f20f6a97dd082c75ad60cc31447bd24b5b3ebe5e64af4ac28712b618",
    "inputs":"aa651d583fb5c8ade1fee0bbf1a9715fee97ec3c4711c8c10c89502556f3bdf2",
    "history":"126880ede7e13358d79d5217c5c0fbcbd0d069bed868af1df62a3a93c38dcaaf",
}


def points(best, score):
    return (2*COEF*best+score)//(2*score) if score>0 else 0


def rank(history, candidate):
    names, rows = [], []
    with history.open(newline="") as f:
        reader=csv.reader(f)
        if next(reader)[:2]!=["min",str(COEF)]:
            raise ValueError("wrong historical formula")
        for name,*row in reader:
            if len(row)!=COUNT:
                raise ValueError("wrong archived case count")
            names.append(name)
            rows.append(array("q",map(int,row)))
    if len(names)!=1299 or len(set(names))!=len(names):
        raise ValueError("wrong archived entrant count")
    archive=tuple(min(s for s in col if s>0) for col in zip(*rows))
    common=tuple(min(m,s) for m,s in zip(archive,candidate,strict=True))
    improved=sum(m!=n for m,n in zip(archive,common,strict=True))
    result={"candidate_case_minima_improved":improved}
    for label, reference in (("fixed_archive",archive),("added",common)):
        ranked=sorted(((sum(points(m,s) for m,s in zip(reference,rs,strict=True)),u)
                      for u,rs in zip(names,rows,strict=True)),reverse=True)
        ours=sum(points(m,s) for m,s in zip(reference,candidate,strict=True))
        if label=="fixed_archive" and ranked[0]!=(2988855588054,"nikaj"):
            raise ValueError(f"archived #1 did not match: {ranked[0]}")
        result[label]={"population":len(names)+(label=="added"),"top_three_archive":ranked[:3],
                       "historical_winner":next(x for x,n in ranked if n=="nikaj"),
                       "candidate":ours,"candidate_rank":1+sum(x>ours for x,_ in ranked),
                       "archive_ties":sum(x==ours for x,_ in ranked)}
    return result


def main():
    p=argparse.ArgumentParser()
    for name in ("cases","output","solver","tester","gen","seeds","inputs","history","gen_metadata"):
        p.add_argument(name,type=lambda x:Path(x).resolve())
    p.add_argument("--source",required=True)
    a=p.parse_args()
    for key,want in HASHES.items():
        if sha(getattr(a,key))!=want:
            p.error(f"wrong official SHA-256 for {key}")
    seeds=a.seeds.read_text().splitlines()
    def csv_rows(path):
        with path.open(newline="") as f:
            return list(csv.DictReader(f))
    features=csv_rows(a.inputs)
    if len(seeds)!=COUNT or len(set(seeds))!=COUNT or features!=csv_rows(a.gen_metadata):
        p.error("archive and official generator metadata differ")
    files=[a.cases/f"{i:04}.txt" for i in range(COUNT)]
    for i, (seed, feature, path) in enumerate(zip(seeds,features,files,strict=True)):
        if feature["file"]!=f"{i:04}" or seed!=feature["seed"]:
            p.error("archive mapping does not match formal seed list")
        head=tuple(map(int,path.read_bytes().split(maxsplit=5)[:5]))
        if head!=tuple(int(feature[k]) for k in ("N","M","Q","L","W")):
            p.error("saved case does not match all published features")
    a.output.mkdir(parents=True,exist_ok=True)
    checkpoint=a.output/"manifest.json"
    meta={"source":a.source,"official":HASHES,
          "programs":{k:sha(v) for k,v in {"solver":a.solver,"gen":a.gen,"tester":a.tester,
             "audit":Path(__file__),"checker":Path(__file__).with_name("benchmark.py")}.items()},
          "cases":{c.name:sha(c) for c in files}}
    if checkpoint.exists() and meta!=json.loads(checkpoint.read_text()):
        p.error("stale checkpoint")
    checkpoint.write_text(json.dumps(meta,indent=2)+"\n")
    raw=[]
    worst=0.
    maximum_queries=0
    total_queries=0
    for i,case in enumerate(files):
        dest=a.output/f"{case.stem}.json"
        if dest.exists():
            row=json.loads(dest.read_text())
        else:
            data=case.read_bytes()
            start=time.monotonic()
            run=subprocess.run([a.tester,a.solver],input=data,capture_output=True,
                               check=True,timeout=2)
            wall=time.monotonic()-start
            if wall>=2:
                raise TimeoutError("tester and solver wall time exceeded 2s")
            detail=score(data,run.stdout)
            actual=detail["score"]
            official=re.fullmatch(rb"Score = (\d+)\s*",run.stderr)
            if actual<=0 or not official or actual!=int(official[1]):
                raise ValueError(f"score disagreement at {case.name}")
            row={"absolute":actual,"queries":detail["queries"],"wall":wall}
            temp=dest.with_suffix(".tmp")
            temp.write_text(json.dumps(row)+"\n")
            temp.replace(dest)
        raw.append(row["absolute"])
        worst=max(worst,row["wall"])
        maximum_queries=max(maximum_queries,row["queries"])
        total_queries+=row["queries"]
        if (i+1)%100==0:
            print(f"{i+1}/{COUNT} legal; official and independent scores agree",flush=True)
    summary={"valid":len(raw),"raw_total":sum(raw),"max_tester_plus_solver_wall":worst,"max_queries":maximum_queries,"queries":total_queries,"archive":rank(a.history,raw)}
    (a.output/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary),flush=True)


if __name__=="__main__":
    main()
