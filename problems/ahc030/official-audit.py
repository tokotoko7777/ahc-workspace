"""Reconstruct official cases from untouched seeds and reweight frozen scores."""
import argparse
import csv
from decimal import Decimal
import hashlib
import json
import os
from pathlib import Path
import re
import signal
import subprocess
import time

from benchmark import replay

PUBLISHED={
    "seeds":"45933552af3c557ff53d4e1397ef9fe10cedea89f10fe8140ceb489569dacea2",
    "inputs":"57f4e664d458d14dc1feab9f24f5fc43e522eab3dab763066f5d47b8be075752",
    "scores":"2ba001dcd7955e7f3607e9eb5e53737dbfa5d61e3e35d34c984ed9e4ba01f252",
}
TOP_USER,TOP_SCORE="terry_u16",2140937483257
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
        assert next(reader)==["min","1000000000","https://img.atcoder.jp/ahc030/awGA15Om.html"]
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
    assert len(order)==1132 and max(order)==(TOP_SCORE,TOP_USER)
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

def checked_tester(tester,binary,case):
    start=time.monotonic()
    with subprocess.Popen([tester,binary],stdin=subprocess.PIPE,stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE,text=True,start_new_session=True) as p:
        try:
            out,err=p.communicate(case,timeout=3)
        except subprocess.TimeoutExpired:
            os.killpg(p.pid,signal.SIGKILL)
            p.communicate()
            raise
        elapsed=time.monotonic()-start
        assert p.returncode==0 and elapsed<3
    score,drills,surveys,guesses,*_=replay(case,out)
    assert re.fullmatch(fr"Score = {score}\s*",err),(score,err)
    assert surveys==0 and guesses==1
    return score,drills,elapsed

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("seeds","inputs","scores","gen","tester","binary","work"):
        p.add_argument(k,type=Path)
    p.add_argument("--prepare-only",action="store_true")
    a=p.parse_args()
    for k in ("seeds","inputs","scores","gen","tester","binary","work"):
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
        src=data.decode().splitlines()
        n,m,eps=src[0].split()
        total=sum(int(line.split()[0]) for line in src[1:1+int(m)])
        assert (n,m,Decimal(eps),total)==(r["N"],r["M"],Decimal(r["eps"]),int(r["total"])),r
        digest.update(data)
    identity={
        "cases_match":CASES,"generator_mode":"untouched official seed list",
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
        writer.writerow(("file","score","drills","seconds"))
        for index,case in enumerate(paths):
            score,drills,t=checked_tester(a.tester,a.binary,case.read_text())
            scores.append(score)
            times.append(t)
            writer.writerow((rows[index]["file"],score,drills,f"{t:.6f}"))
            f.flush()
            if index%500==0:print(f"scored={index+1}/{CASES}",flush=True)
    joint=[min(m,x) for m,x in zip(minima,scores,strict=True)]
    table={user:relative(row,joint) for user,row in score_rows(a.scores)}
    best=max((score,user) for user,score in table.items())
    candidate=relative(scores,joint)
    result=dict(
        **identity,valid_correct_first_attempt=len(scores),max_wall_seconds=max(times),
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
