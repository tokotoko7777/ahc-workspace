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
    "seeds":"d88ec61a00aa2dfb0be89801a7320e56550cf9c71cc2e6959f79c5eb057cf9ae",
    "inputs":"50bfe8fa4be3e043f924c9bb6797fa78db9d0fc28c4db3f5a270934d77ecf7d1",
    "scores":"edde18d960f3f47b2fad0ffa8e539274ce2d18797b73dca7d30c49d179866a82",
}
TOP_USER,TOP_SCORE="kurakura",1893330847366
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
        assert next(reader)==["min","1000000000","https://img.atcoder.jp/ahc063/O25rQjiK.html"]
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
    assert len(order)==1268 and max(order)==(TOP_SCORE,TOP_USER)
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
    score,expanded,_,_=replay(data,run.stdout)
    out_path.write_bytes(run.stdout)
    judged=subprocess.run([vis,case,out_path],cwd=out_path.parent,capture_output=True,
                          text=True,timeout=2,check=True)
    assert judged.stdout.strip()==f"Score = {score}",(case.name,judged.stdout)
    m=re.fullmatch(rb"literal=(\d+) base=(\d+) score=(\d+) expanded=(\d+) frozen=(\d+) trials=10\s*",run.stderr)
    assert m and (int(m[3]),int(m[4]))==(score,expanded)
    return score,expanded,elapsed

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("seeds","inputs","scores","gen","traced","vis","binary","work"):
        p.add_argument(k,type=Path)
    p.add_argument("--freeze-id",required=True)
    p.add_argument("--prepare-only",action="store_true")
    a=p.parse_args()
    for k in ("seeds","inputs","scores","gen","traced","vis","binary","work"):
        setattr(a,k,getattr(a,k).resolve())
    a.work.mkdir(parents=True,exist_ok=True)
    (a.work/"summary.json").unlink(missing_ok=True)
    rows=identity_metadata(a)
    minima=baseline(a.scores)
    subprocess.run([a.gen,a.seeds],cwd=a.work,check=True)
    trace=subprocess.run([a.traced,a.seeds,"--dir","w-check"],cwd=a.work,check=True,capture_output=True)
    expected=[f"AHC066_W,{r['seed']},{r['W']}" for r in rows]
    assert trace.stderr.decode().splitlines()==expected
    paths=[a.work/"in"/(r["file"]+".txt") for r in rows]
    digest=hashlib.sha256()
    for case,r in zip(paths,rows,strict=True):
        data=case.read_bytes()
        assert data==(a.work/"w-check"/case.name).read_bytes(),case.name
        assert data.decode().splitlines()[0].split()==[r[k] for k in ("N","M","T")],r
        digest.update(data)
    identity={
        "frozen_git_commit":a.freeze_id,"cases_match":CASES,"W_matched_trace_and_stock_bytes":CASES,"generator_mode":"untouched official seed list",
        "ordered_cases_sha256":digest.hexdigest(),
        "sources_sha256":PUBLISHED,"generator_sha256":sha(a.gen),"traced_generator_sha256":sha(a.traced),"visualizer_sha256":sha(a.vis),
        "binary_sha256":sha(a.binary),"historical_winner":TOP_USER,"historical_winner_score":TOP_SCORE,
    }
    atomic_json(a.work/"identity.json",identity)
    print(json.dumps(identity),flush=True)
    if a.prepare_only:return
    scores,times=[],[]
    with (a.work/"checked-scores.csv").open("w",newline="") as f:
        writer=csv.writer(f,lineterminator="\n")
        writer.writerow(("file","score","expanded","seconds"))
        for index,case in enumerate(paths):
            score,expanded,t=checked(a.vis,a.binary,case,a.work/"answer.txt")
            scores.append(score)
            times.append(t)
            writer.writerow((rows[index]["file"],score,expanded,f"{t:.6f}"))
            f.flush()
            if index%500==0:print(f"scored={index+1}/{CASES}",flush=True)
    joint=[min(m,x) for m,x in zip(minima,scores,strict=True)]
    table={user:relative(row,joint) for user,row in score_rows(a.scores)}
    best=max((score,user) for user,score in table.items())
    candidate=relative(scores,joint)
    result=dict(
        **identity,valid_cases=len(scores),max_wall_seconds=max(times),
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
