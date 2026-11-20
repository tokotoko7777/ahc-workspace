#!/usr/bin/env python3
"""Measure a frozen admissions policy on the published 2000-case official population."""
import argparse
from array import array
import csv
from decimal import Decimal
import json
from pathlib import Path
from benchmark import evaluate, sha

N = 2000
COUNT = 1128
SCALE = 10**9
FIRST = ("asi1024", 1981252894878)
HASHES = {
    "seeds": "f63752a346a388e810574d22c9ee271f4f77d0b5f9e353084f2c335417c96482",
    "metadata": "53a2f72e96d7b9b40f3968797422cb4c7ce849a3a7eaeeae720a2fc5e0ee0fa5",
    "historical": "786a2204ffee292478e225dfed315024266eaa27b648287047a2955417cc25c9",
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
    for key in ("seeds","metadata","historical","cases","instrumented_cases","instrumented_log","tester","solver","output"):
        parser.add_argument(key,type=lambda p: Path(p).resolve())
    a=parser.parse_args()
    for name,h in HASHES.items():
        if sha(getattr(a,name)) != h:
            raise ValueError(f"wrong official {name}")
    official_seeds=[int(x) for x in a.seeds.read_text().split()]
    with a.metadata.open() as handle:
        reader=csv.DictReader(handle)
        assert reader.fieldnames == ["file","seed","R","num_cluster","num_pond"]
        metadata=list(reader)
    if (len(official_seeds)!=N or [int(row["seed"]) for row in metadata]!=official_seeds
        or [row["file"] for row in metadata]!=[f"{i:04d}" for i in range(N)]):
        raise ValueError("official seed or filename alignment differs")
    cases=sorted(a.cases.glob("*.txt"))
    if [case.stem for case in cases]!=[row["file"] for row in metadata]:
        raise ValueError("generated filenames mismatch")
    observed=list(csv.reader(a.instrumented_log.open()))
    if len(observed)!=N:
        raise ValueError("instrumented generation metadata incomplete")
    case_hashes={}
    for case,row,features in zip(cases,metadata,observed):
        with case.open() as f:
            n,m,r=f.readline().split()
            total_pond=sum(f.readline().count("#") for _ in range(50))
        expected=[int(row["seed"]),Decimal(row["R"]),int(row["num_cluster"]),int(row["num_pond"])]
        actual=[int(features[0]),Decimal(features[1]),int(features[2]),int(features[3])]
        if (expected!=actual or (n,m)!=("50","1000") or Decimal(r)!=expected[1] or
            total_pond!=expected[2]+expected[3]):
            raise ValueError("published generation metadata mismatches")
        digest=sha(case)
        if digest!=sha(a.instrumented_cases/case.name):
            raise ValueError("metadata instrumentation changed an official input")
        case_hashes[case.name]=digest
    with a.historical.open() as handle:
        reader=csv.reader(handle)
        header=next(reader)
        if header[:2]!=["max",str(SCALE)]:
            raise ValueError("historical header mismatch")
        entries=[(r[0],array("q",(max(0,int(s)) for s in r[1:]))) for r in reader]
    if len(entries)!=COUNT or any(len(values)!=N for _,values in entries):
        raise ValueError("historical absolute-score matrix shape mismatch")
    maxima=[max(v) for v in zip(*(scores for _,scores in entries))]
    if max(((user,relative(v,maxima)) for user,v in entries),key=lambda x:x[1])!=FIRST:
        raise ValueError("historical winner mismatch")

    a.output.mkdir(parents=True,exist_ok=True)
    manifest={"official":HASHES,"tester":sha(a.tester),"solver":sha(a.solver),
              "runner":sha(Path(__file__)),"replay":sha(Path(__file__).with_name("benchmark.py")),
              "cases":case_hashes,"instrumented_log":sha(a.instrumented_log)}
    man=a.output/"manifest.json"
    if man.exists() and json.loads(man.read_text())!=manifest:
        raise ValueError("stale checkpoint")
    man.write_text(json.dumps(manifest,indent=2)+"\n")
    print(f"Matched ordered official seeds, R/pond generation, input bytes, and official ranking: {N}/{N} before execution",flush=True)
    checkpoint=a.output/"checkpoints"
    checkpoint.mkdir(exist_ok=True)
    own=[]; max_wall=0.; accepted=moved=diff_count=max_diff=0
    for i,case in enumerate(cases,1):
        row=checkpoint/f"{case.stem}.json"
        if row.exists():
            result=json.loads(row.read_text())
        else:
            result=evaluate(case,a.tester,a.solver,allow_approx_internal=True)
            tmp=row.with_suffix(".tmp")
            tmp.write_text(json.dumps(result)+"\n")
            tmp.replace(row)
        own.append(result["score"])
        max_wall=max(max_wall,result["wall"])
        accepted+=result["accepted"];moved+=result["moved"]
        diff_count+=result["internal_diff"]!=0
        max_diff=max(max_diff,abs(result["internal_diff"]))
        if i%250==0:
            print(f"{i}/{N} legal and exact",flush=True)
    combined=[max(x,y) for x,y in zip(maxima,own)]
    summary={"legal":len(own),"raw":sum(own),"max_wall":max_wall,"accepted":accepted,"moved":moved,
             "updated_maxima":sum(a!=b for a,b in zip(maxima,combined)),
             "internal_rounding_count":diff_count,"internal_rounding_max_abs":max_diff,
             "historical_fixed":comparison(entries,own,maxima),
             "hypothetical_shared":comparison(entries,own,combined)}
    (a.output/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary),flush=True)


if __name__=="__main__":
    main()
