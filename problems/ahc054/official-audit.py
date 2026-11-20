"""Reconstruct official cases from untouched seeds and reweight frozen scores."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time

from benchmark import replay

PUBLISHED={
    "seeds":"f46e04db0ec8f80d641bbc8e166b1d2f4050a340b356acea1adce1a41a0a7fc8",
    "inputs":"12a79763705f3118ee2f2ce80c50a6f0a3569e89b251f98b173bc338048d3ba9",
    "scores":"50617ebf3ae1d3a70dd27b4c712e61d72543b6a58ba7816e2fee6039dade0293",
}
TOP_USER,TOP_SCORE="saharan",2409003089874
CASES=3000

def sha(path):
    digest=hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda:stream.read(1<<20),b""):
            digest.update(chunk)
    return digest.hexdigest()

def score_rows(path):
    with path.open(newline="") as stream:
        reader=csv.reader(stream)
        assert next(reader)==["max","1000000000","https://img.atcoder.jp/ahc054/YDAxDRZr_v2.html"]
        for user,*cells in reader:
            assert len(cells)==CASES
            v=list(map(int,cells))
            assert 0 not in v
            yield user,v

def relative(raw,maxima):
    return sum((2*10**9*a+b)//(2*b) for a,b in zip(raw,maxima,strict=True) if a>0)

def baseline(path):
    maxima=[0]*CASES
    for _,row in score_rows(path):
        maxima=[max(m,x) for x,m in zip(row,maxima,strict=True)]
    assert all(m>0 for m in maxima)
    order=[(relative(raw,maxima),user) for user,raw in score_rows(path)]
    assert len(order)==1190 and max(order)==(TOP_SCORE,TOP_USER)
    return maxima

def identity_metadata(a):
    assert all(sha(getattr(a,k))==h for k,h in PUBLISHED.items())
    with a.inputs.open(newline="") as stream:
        rows=list(csv.DictReader(stream))
    assert len(rows)==CASES
    assert [r["file"] for r in rows]==[f"{i:04}" for i in range(CASES)]
    assert [r["seed"] for r in rows]==a.seeds.read_text().splitlines()
    assert len({r["seed"] for r in rows})==CASES
    return rows

def atomic_json(path,value):
    temp=path.with_suffix(".tmp")
    temp.write_text(json.dumps(value,indent=2)+"\n")
    temp.replace(path)

def checked(tester,binary,case):
    data=case.read_bytes()
    start=time.monotonic()
    run=subprocess.run([tester,binary],input=data,capture_output=True,timeout=2,check=True)
    elapsed=time.monotonic()-start
    assert elapsed<2
    score,planted,path=replay(data,run.stdout)
    m=re.fullmatch(rb"path=(\d+)\s+Score = (\d+)\s*",run.stderr)
    assert m and (int(m[1]),int(m[2]))==(path,score) and score>=path
    return score,planted,path,elapsed

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("seeds","inputs","scores","gen","tester","binary","work"):
        p.add_argument(k,type=Path)
    p.add_argument("--freeze-id",required=True)
    p.add_argument("--prepare-only",action="store_true")
    a=p.parse_args()
    for k in ("seeds","inputs","scores","gen","tester","binary","work"):
        setattr(a,k,getattr(a,k).resolve())
    a.work.mkdir(parents=True,exist_ok=True)
    (a.work/"summary.json").unlink(missing_ok=True)
    rows=identity_metadata(a)
    maxima=baseline(a.scores)
    subprocess.run([a.gen,a.seeds],cwd=a.work,check=True)
    paths=[a.work/"in"/(r["file"]+".txt") for r in rows]
    digest=hashlib.sha256()
    for case,r in zip(paths,rows,strict=True):
        data=case.read_bytes()
        lines=data.splitlines()
        n=int(lines[0].split()[0])
        assert (n,sum(line.count(b"T") for line in lines[1:1+n]))==(int(r["N"]),int(r["K"])),r
        digest.update(data)
    identity={
        "frozen_git_commit":a.freeze_id,"cases_match":CASES,"generator_mode":"untouched official seed list",
        "ordered_cases_sha256":digest.hexdigest(),
        "sources_sha256":PUBLISHED,"generator_sha256":sha(a.gen),"tester_sha256":sha(a.tester),
        "binary_sha256":sha(a.binary),"historical_winner":TOP_USER,"historical_winner_score":TOP_SCORE,
    }
    atomic_json(a.work/"identity.json",identity)
    print(json.dumps(identity),flush=True)
    if a.prepare_only:return
    scores,times=[],[]
    with (a.work/"checked-scores.csv").open("w",newline="") as f:
        writer=csv.writer(f,lineterminator="\n")
        writer.writerow(("file","score","planted","path","seconds"))
        for index,case in enumerate(paths):
            score,planted,path,t=checked(a.tester,a.binary,case)
            scores.append(score)
            times.append(t)
            writer.writerow((rows[index]["file"],score,planted,path,f"{t:.6f}"))
            f.flush()
            if index%500==0:print(f"scored={index+1}/{CASES}",flush=True)
    joint=[max(m,x) for m,x in zip(maxima,scores,strict=True)]
    table={user:relative(row,joint) for user,row in score_rows(a.scores)}
    best=max((score,user) for user,score in table.items())
    candidate=relative(scores,joint)
    result=dict(
        **identity,valid_cases=len(scores),max_wall_seconds=max(times),
        total_absolute_score=sum(scores),case_maxima_improvement_count=sum(x>m for x,m in zip(scores,maxima,strict=True)),
        own_fixed_historical_baseline=relative(scores,maxima),own_combined_baseline=candidate,
        archived_first_combined_baseline=table[TOP_USER],best_historical_user_after_combining=best[1],
        best_historical_score_after_combining=best[0],
        hypothetical_rank=1+sum(s>candidate for s in table.values()),
        hypothetical_ties=sum(s==candidate for s in table.values()),
    )
    atomic_json(a.work/"summary.json",result)
    print(json.dumps(result),flush=True)

if __name__=="__main__":
    main()
