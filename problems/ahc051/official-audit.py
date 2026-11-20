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
    "seeds":"b61fb5678d480a79e25a50141403e14e8e245d5699e90aa53d4fad893e10dee8",
    "inputs":"1e3708bf9507f664c83dc1c4ac4cb4ec065739eb712be1f1ddce65c0c43e51d8",
    "scores":"7c1b931d4c62deb3fd63d1a3401e398e9ecba0bffdc2d3439d6f58b2bf0ec838",
}
TOP_USER,TOP_SCORE="eijirou",1721504233142
CASES=2000

def sha(path):
    digest=hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda:stream.read(1<<20),b""):
            digest.update(chunk)
    return digest.hexdigest()

def score_rows(path):
    with path.open(newline="") as stream:
        reader=csv.reader(stream)
        assert next(reader)==["min","1000000000","https://img.atcoder.jp/ahc051/jdd9gfQC.html"]
        for user,*cells in reader:
            assert len(cells)==CASES
            v=list(map(int,cells))
            assert 0 not in v
            yield user,v

def relative(raw,minima):
    return sum((2*10**9*b+a)//(2*a) for a,b in zip(raw,minima,strict=True) if a>0)

def baseline(path):
    minima=[10**9]*CASES
    for _,row in score_rows(path):
        minima=[min(m,x) if x>0 else m for x,m in zip(row,minima,strict=True)]
    assert all(m>0 for m in minima)
    order=[(relative(raw,minima),user) for user,raw in score_rows(path)]
    assert len(order)==1183 and max(order)==(TOP_SCORE,TOP_USER)
    return minima

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

def checked(vis,binary,case,out_path):
    data=case.read_bytes()
    start=time.monotonic()
    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
    elapsed=time.monotonic()-start
    assert elapsed<2
    score,reachable=replay(data.splitlines(),run.stdout)
    out_path.write_bytes(run.stdout)
    judged=subprocess.run([vis,case,out_path],cwd=out_path.parent,capture_output=True,
                          text=True,timeout=2,check=True)
    assert judged.stdout.strip()==f"Score = {score}",(case.name,judged.stdout)
    m=re.fullmatch(rb"base=(\d+) final=(\d+) frozen=(\d+)\s*",run.stderr)
    assert m and int(m[2])==score
    return score,reachable,elapsed

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("seeds","inputs","scores","gen","vis","binary","work"):
        p.add_argument(k,type=Path)
    p.add_argument("--freeze-id",required=True)
    p.add_argument("--prepare-only",action="store_true")
    a=p.parse_args()
    for k in ("seeds","inputs","scores","gen","vis","binary","work"):
        setattr(a,k,getattr(a,k).resolve())
    a.work.mkdir(parents=True,exist_ok=True)
    (a.work/"summary.json").unlink(missing_ok=True)
    rows=identity_metadata(a)
    minima=baseline(a.scores)
    subprocess.run([a.gen,a.seeds],cwd=a.work,check=True)
    paths=[a.work/"in"/(r["file"]+".txt") for r in rows]
    digest=hashlib.sha256()
    for case,r in zip(paths,rows,strict=True):
        data=case.read_bytes()
        assert data.decode().splitlines()[0].split()==[r[k] for k in ("N","M","K")],r
        digest.update(data)
    identity={
        "frozen_git_commit":a.freeze_id,"cases_match":CASES,"generator_mode":"untouched official seed list",
        "ordered_cases_sha256":digest.hexdigest(),
        "sources_sha256":PUBLISHED,"generator_sha256":sha(a.gen),"visualizer_sha256":sha(a.vis),
        "binary_sha256":sha(a.binary),"historical_winner":TOP_USER,"historical_winner_score":TOP_SCORE,
    }
    atomic_json(a.work/"identity.json",identity)
    print(json.dumps(identity),flush=True)
    if a.prepare_only:return
    scores,times=[],[]
    all_processors=0
    with (a.work/"checked-scores.csv").open("w",newline="") as f:
        writer=csv.writer(f,lineterminator="\n")
        writer.writerow(("file","score","reachable_processors","seconds"))
        for index,case in enumerate(paths):
            score,reachable,t=checked(a.vis,a.binary,case,a.work/"answer.txt")
            all_processors+=reachable==int(rows[index]["N"])
            scores.append(score)
            times.append(t)
            writer.writerow((rows[index]["file"],score,reachable,f"{t:.6f}"))
            f.flush()
            if index%500==0:print(f"scored={index+1}/{CASES}",flush=True)
    joint=[min(m,x) for m,x in zip(minima,scores,strict=True)]
    table={user:relative(row,joint) for user,row in score_rows(a.scores)}
    best=max((score,user) for user,score in table.items())
    candidate=relative(scores,joint)
    result=dict(
        **identity,valid_cases=len(scores),all_processors_reached_cases=all_processors,max_wall_seconds=max(times),
        total_absolute_score=sum(scores),case_minima_improvement_count=sum(x<m for x,m in zip(scores,minima,strict=True)),
        own_fixed_historical_baseline=relative(scores,minima),own_combined_baseline=candidate,
        archived_first_combined_baseline=table[TOP_USER],best_historical_user_after_combining=best[1],
        best_historical_score_after_combining=best[0],
        hypothetical_rank=1+sum(s>candidate for s in table.values()),
        hypothetical_ties=sum(s==candidate for s in table.values()),
    )
    atomic_json(a.work/"summary.json",result)
    print(json.dumps(result),flush=True)

if __name__=="__main__":
    main()
