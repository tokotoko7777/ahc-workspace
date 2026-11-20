#!/usr/bin/env python3
"""Blind, frozen one-ply evaluation on 3000 official public system seeds."""
import argparse
from array import array
import csv
import json
from pathlib import Path
from benchmark import evaluate, sha

N = 3000
COUNT = 1171
SCALE = 10**9
FIRST = ("cuthbert", 2424667397941)
HASHES = {
    "seeds": "24432b101519407fc0e5c2f92f3b939089115b79fd2e71ab3ee737ebc53e0601",
    "metadata": "4d0ae86fb6a35f0ec06d94ba2d6126a19958ac85e4a44d752eac15a91cf89efa",
    "historical": "aeb7fcde7cfd8ec5437fd4290ab3ed7e33c584e8a207281936b84ecbfeed41cf",
}


def relative(values, maxima):
    return sum((2 * SCALE * s + m) // (2 * m) if m else 0 for s, m in zip(values, maxima))


def comparison(entries, ours, maxima):
    historical = [(user, relative(scores, maxima)) for user, scores in entries]
    ours_score = relative(ours, maxima)
    winner = max(historical, key=lambda x: x[1])
    return {"best_historical": winner, "self": ours_score,
            "self_rank": 1 + sum(score > ours_score for _, score in historical),
            "self_ties": sum(score == ours_score for _, score in historical)}


def main():
    parser = argparse.ArgumentParser()
    for key in ("seeds","metadata","historical","cases","tester","solver","output"):
        parser.add_argument(key,type=lambda p: Path(p).resolve())
    a=parser.parse_args()
    for name,h in HASHES.items():
        if sha(getattr(a,name)) != h:
            raise ValueError(f"wrong official {name}")
    official_seeds=[int(x) for x in a.seeds.read_text().split()]
    with a.metadata.open() as handle:
        reader=csv.DictReader(handle)
        assert reader.fieldnames == ["file","seed","M","U"]
        metadata=list(reader)
    if (len(official_seeds)!=N or [int(row["seed"]) for row in metadata]!=official_seeds
        or [row["file"] for row in metadata]!=[f"{i:04d}" for i in range(N)]):
        raise ValueError("official seed or filename alignment differs")
    cases=sorted(a.cases.glob("*.txt"))
    if [case.stem for case in cases]!=[row["file"] for row in metadata]:
        raise ValueError("generated filenames mismatch")
    for case,row in zip(cases,metadata):
        with case.open() as fh:
            n,m,t,u=map(int,fh.readline().split())
        if (n,m,t,u)!=(10,int(row["M"]),100,int(row["U"])):
            raise ValueError("generated dimensions do not match archived input metadata")
    with a.historical.open() as handle:
        reader=csv.reader(handle)
        header=next(reader)
        if header[:2]!=["max",str(SCALE)]:
            raise ValueError("historical header mismatch")
        entries=[(r[0],array("i",(max(0,int(s)) for s in r[1:]))) for r in reader]
    if len(entries)!=COUNT or any(len(values)!=N for _,values in entries):
        raise ValueError("historical absolute-score matrix shape mismatch")
    maxima=[max(v) for v in zip(*(scores for _,scores in entries))]
    if max(((user,relative(v,maxima)) for user,v in entries),key=lambda x:x[1])!=FIRST:
        raise ValueError("historical winner mismatch")

    a.output.mkdir(parents=True,exist_ok=True)
    manifest={"official":HASHES,"tester":sha(a.tester),"solver":sha(a.solver),
              "runner":sha(Path(__file__)),"replay":sha(Path(__file__).with_name("benchmark.py")),
              "cases":{c.name:sha(c) for c in cases}}
    man=a.output/"manifest.json"
    if man.exists() and json.loads(man.read_text())!=manifest:
        raise ValueError("stale checkpoint")
    man.write_text(json.dumps(manifest,indent=2)+"\n")
    print(f"Matched ordered official seeds, M/U and official ranking: {N}/{N} before execution",flush=True)
    checkpoint=a.output/"checkpoints"
    checkpoint.mkdir(exist_ok=True)
    own=[]; max_wall=0.
    for i,case in enumerate(cases,1):
        row=checkpoint/f"{case.stem}.json"
        if row.exists():
            result=json.loads(row.read_text())
        else:
            result=evaluate(case,a.tester,a.solver)
            tmp=row.with_suffix(".tmp")
            tmp.write_text(json.dumps(result)+"\n")
            tmp.replace(row)
        own.append(result["score"])
        max_wall=max(max_wall,result["wall"])
        if i%250==0:
            print(f"{i}/{N} legal and exact",flush=True)
    combined=[max(x,y) for x,y in zip(maxima,own)]
    summary={"legal":len(own),"raw":sum(own),"max_wall":max_wall,
             "updated_maxima":sum(a!=b for a,b in zip(maxima,combined)),
             "historical_fixed":comparison(entries,own,maxima),
             "hypothetical_shared":comparison(entries,own,combined)}
    (a.output/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary),flush=True)


if __name__=="__main__":
    main()
