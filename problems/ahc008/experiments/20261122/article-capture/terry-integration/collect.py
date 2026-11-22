"""Collect one label row per generated case, all five forced maps per case.
Official tester, no practice/system inputs, a unique and resumable data folder.
"""
import argparse
import csv
import hashlib
import json
import pathlib
import re
import subprocess
import time

MAPS = 5
FIELDS = ["case_id","seed","input_sha256","M","N","cow","pig","rabbit","dog","cat"]
FIELDS += [f"score_{k}" for k in range(MAPS)] + [f"seconds_{k}" for k in range(MAPS)]

def digest(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()

def seed(id):
    key = f"ahc008/terry-five-map/offline-selector/pilot-v1/{id}".encode()
    return int.from_bytes(hashlib.sha256(key).digest()[:8], "little")

def features(raw):
    lines = list(map(str.split, raw.decode("ascii").splitlines()))
    n = int(lines[0][0])
    m = int(lines[n+1][0])
    ct = [0]*5
    for (_, _, t) in lines[1:n+1]:
        ct[int(t)-1] += 1
    if sum(ct) != n or not 5 <= m <= 10 or not 10 <= n <= 20:
        raise ValueError("unexpected official input")
    return [m,n]+ct

def main():
    p=argparse.ArgumentParser()
    p.add_argument("gen",type=pathlib.Path)
    p.add_argument("tester",type=pathlib.Path)
    p.add_argument("integrated_exe",type=pathlib.Path)
    p.add_argument("out",type=pathlib.Path)
    p.add_argument("--split",choices=("train","validation"),required=True)
    p.add_argument("--count",type=int)
    p.add_argument("--start",type=int)
    p.add_argument("--timeout",type=float,default=5)
    a=p.parse_args()
    lo,hi = (33000,34000) if a.split=="train" else (34000,35000)
    start = lo if a.start is None else a.start
    count = (80 if a.split=="train" else 20) if a.count is None else a.count
    if not (lo <= start < start+count <= hi):
        p.error(f"{a.split} case IDs must be between {lo} and {hi-1}, with positive count")
    gen,tester,solver,out=(getattr(a,k).resolve() for k in ("gen","tester","integrated_exe","out"))
    out.mkdir(parents=True,exist_ok=True)
    ids=range(start,start+count)
    seeds=[seed(i) for i in ids]
    manifest={
        "schema":"ahc008-terry-five-map-v1",
        "split":a.split, "start":start, "count":count,
        "seed_namespace":"ahc008/terry-five-map/offline-selector/pilot-v1",
        "gen_sha256":digest(gen),"tester_sha256":digest(tester),"solver_sha256":digest(solver),
        "score_scale":100_000_000,"forced_maps":list(range(MAPS)),
    }
    mpath=out/"manifest.json"
    if mpath.exists():
        old=json.loads(mpath.read_text())
        if any(old.get(k)!=v for k,v in manifest.items()):
            raise SystemExit("resume refused: metadata (suite/binaries) changed")
    else:
        if (out/"labels.csv").exists() or (out/"in").exists():
            raise SystemExit("output folder has data but no manifest")
        mpath.write_text(json.dumps(manifest,indent=2)+"\n")
    (out/"seeds.txt").write_text("".join(f"{x}\n" for x in seeds))
    subprocess.run([gen,"seeds.txt"],cwd=out,check=True)
    csvpath=out/"labels.csv"
    done={}
    if csvpath.exists():
        with csvpath.open(newline="") as f:
            for row in csv.DictReader(f):
                i=int(row["case_id"])
                if i in done or i not in ids or int(row["seed"])!=seeds[i-start]:
                    raise SystemExit("resume refused: inconsistent stored case row")
                done[i]=row
    calibrated_map=re.compile(rb"terry_forced_map\s*=\s*([0-4])\b")
    for j,i in enumerate(ids):
        raw=(out/"in"/f"{j:04d}.txt").read_bytes()
        sha=hashlib.sha256(raw).hexdigest()
        if i in done:
            if done[i]["input_sha256"]!=sha:
                raise SystemExit("resume refused: regenerated input changed")
            continue
        scores=[];seconds=[]
        for k in range(MAPS):
            t=time.monotonic()
            r=subprocess.run([tester,solver,f"--terry-force-map={k}"],input=raw,
                stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,timeout=a.timeout)
            scores_seen=re.findall(rb"Score = ([0-9]+)",r.stderr)
            maps_seen=calibrated_map.findall(r.stderr)
            if r.returncode or len(scores_seen)!=1 or maps_seen!=[str(k).encode()]:
                raise RuntimeError(f"case {i}, map {k}: missing calibrated map or official score; {r.stderr[-2000:]!r}")
            s=int(scores_seen[0])
            if not 0<=s<=100_000_000:
                raise RuntimeError(f"case {i}, map {k}: invalid score {s}")
            scores.append(s);seconds.append(f"{time.monotonic()-t:.6f}")
        new=not csvpath.exists() or not csvpath.stat().st_size
        with csvpath.open("a",newline="") as f:
            w=csv.writer(f)
            if new:w.writerow(FIELDS)
            w.writerow([i,seeds[j],sha]+features(raw)+scores+seconds)
        print(i,*scores,flush=True)
    manifest["labels_sha256"]=digest(csvpath)
    manifest["cases_collected"]=count
    tmp=mpath.with_suffix(".tmp")
    tmp.write_text(json.dumps(manifest,indent=2)+"\n")
    tmp.replace(mpath)

if __name__=="__main__":
    main()
