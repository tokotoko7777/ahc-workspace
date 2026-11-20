"""Reconstruct the official final corpus, score one frozen binary, and audit both baselines."""
import argparse
from collections import Counter
import csv
import hashlib
import json
from pathlib import Path
import re
import subprocess

from benchmark import run, validate

PUBLISHED = {
    "seeds": "2154215b63e063812cf2bb0ee2370c5193ae4682a3f7ce260e82a52a07663206",
    "inputs": "144726d1aed0310736ffcdc74be1ebddc0e712e45bba355d861bb728f6e095cd",
    "scores": "5864a1b7c0699e66db295ed12611c53567e944f5bbd8965621ac8648b34e2cf1",
}
TOP_USER, TOP_SCORE = "Jirotech", 2808216532758
COUNT = 3000

def sha(path):
    digest=hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda:stream.read(1<<20),b""):
            digest.update(chunk)
    return digest.hexdigest()

def metadata(a):
    assert all(sha(getattr(a,k))==v for k,v in PUBLISHED.items())
    with a.inputs.open(newline="") as stream:
        rows=list(csv.DictReader(stream))
    assert len(rows)==COUNT
    published=a.seeds.read_text().splitlines()
    assert len(published)==COUNT
    assert [r["file"] for r in rows]==[f"{i:04}" for i in range(COUNT)]
    assert [r["seed"] for r in rows]==published, "score metadata and published seed sequence differ"
    assert len(set(published))==COUNT
    assert Counter(int(r["S"]) for r in rows)=={i*i:100 for i in range(1,31)}
    return rows

def score_rows(path):
    with path.open(newline="") as stream:
        reader=csv.reader(stream)
        assert next(reader)==["max","","https://img.atcoder.jp/ahc022/u4u4grds7n.html"]
        for user,*cells in reader:
            assert len(cells)==COUNT
            yield user, list(map(int,cells))

def rounded(scores,maxima):
    return sum((2*10**9*x+m)//(2*m) for x,m in zip(scores,maxima,strict=True) if x>0)

def official_baseline(path):
    maxima=[0]*COUNT
    for _,score in score_rows(path):
        maxima=[max(a,b) for a,b in zip(maxima,score,strict=True)]
    assert min(maxima)>0
    totals=[(rounded(v,maxima),user) for user,v in score_rows(path)]
    assert len(totals)==1063 and max(totals)==(TOP_SCORE,TOP_USER)
    return maxima

def replace_json(path, value):
    temporary=path.with_suffix(path.suffix+".tmp")
    temporary.write_text(json.dumps(value,indent=2)+"\n")
    temporary.replace(path)

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for name in ("seeds","inputs","scores","gen","tester","solver","work"):
        ap.add_argument(name,type=Path)
    ap.add_argument("--prepare-only",action="store_true")
    a=ap.parse_args()
    for k in ("seeds","inputs","scores","gen","tester","solver","work"):
        setattr(a,k,getattr(a,k).resolve())
    a.work.mkdir(parents=True,exist_ok=True)
    (a.work/"summary.json").unlink(missing_ok=True)
    rows=metadata(a)
    maxima=official_baseline(a.scores)
    # Use the untouched published seed file as documented by the tools.
    # Do not override S: the corresponding RNG draw advances the stream
    # before generating coordinates, the permutation, and query noise.
    subprocess.run([a.gen,a.seeds],cwd=a.work,check=True)
    inputs=[a.work/"in"/f'{r["file"]}.txt' for r in rows]
    digest=hashlib.sha256()
    for path,r in zip(inputs,rows,strict=True):
        data=path.read_bytes()
        header=tuple(map(int,data.splitlines()[0].split()))
        assert header==tuple(int(r[k]) for k in ("L","N","S")),(path,header,r)
        assert len(data.split())==3+3*header[1]+10000
        digest.update(data)
    identity={
        "case_count":len(inputs), "ordered_case_bytes_sha256":digest.hexdigest(),
        "generator_mode":"published seed file only; no parameter overrides",
        "sources_sha256":PUBLISHED,
        "generator_sha256":sha(a.gen),"tester_sha256":sha(a.tester),"solver_sha256":sha(a.solver),
        "archived_first_user":TOP_USER,"archived_first_score":TOP_SCORE,
    }
    replace_json(a.work/"identity.json",identity)
    print(json.dumps(identity),flush=True)
    if a.prepare_only:
        return
    raw=[]
    errors=Counter()
    times=[]
    with (a.work/"checked-scores.csv").open("w",newline="") as stream:
        w=csv.writer(stream,lineterminator="\n")
        w.writerow(("file","absolute","errors","placement","measure_cost","queries","L","N","S","seconds"))
        for i,path in enumerate(inputs):
            case=path.read_bytes()
            out,observed,elapsed=run(a.tester,a.solver,case)
            data=validate(case,out)
            m=re.fullmatch(rb"Score = (\d+)\nNumber of wrong answers = (\d+)"
                          rb"\nPlacement cost = (\d+)\nMeasurement cost = (\d+)"
                          rb"\nMeasurement count = (\d+)\s*",observed)
            assert m and tuple(map(int,m.groups()))==data[:5],(path,observed,data)
            if not i%250:
                print(f"scored={i+1}/{len(inputs)}",flush=True)
            times.append(elapsed)
            raw.append(data[0])
            errors[data[1]]+=1
            w.writerow((rows[i]["file"],*data,f"{elapsed:.6f}"))
            stream.flush()
    combined=[max(a,b) for a,b in zip(raw,maxima,strict=True)]
    totals={user:rounded(v,combined) for user,v in score_rows(a.scores)}
    archive_best=max((score,user) for user,score in totals.items())
    my_score=rounded(raw,combined)
    final=dict(
        **identity, own_cases=len(raw), errors=dict(sorted(errors.items())),
        max_wall_seconds=max(times), total_absolute_score=sum(raw),
        own_over_historical_case_max=sum(a>b for a,b in zip(raw,maxima,strict=True)),
        own_fixed_historical_baseline=rounded(raw,maxima),
        own_combined_baseline=my_score,
        archived_first_combined_baseline=totals[TOP_USER],
        best_historical_user_after_combining=archive_best[1],
        best_historical_score_after_combining=archive_best[0],
        new_rank=1+sum(v>my_score for v in totals.values()),
        tied_archived_participants=sum(v==my_score for v in totals.values()),
    )
    replace_json(a.work/"summary.json",final)
    print(json.dumps(final),flush=True)

if __name__=="__main__":
    main()
