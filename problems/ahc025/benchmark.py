#!/usr/bin/env python3
"""Official lock-step tester and independent transcript/protocol/raw variance check."""
import argparse
import hashlib
import math
import json
import re
import subprocess
import time
from pathlib import Path


def score(case,transcript):
    data=list(map(int,case.read_text().split()))
    n,d,q=data[:3]
    weights=data[3:]
    if len(weights)!=n:
        raise ValueError("malformed generated case")
    lines=transcript.splitlines()
    if len(lines)!=q+1:
        raise ValueError("wrong interactive query count")
    for line in lines[:-1]:
        left,right,*ids=map(int,line.split())
        if (min(left,right)<1 or len(ids)!=left+right or
            len(set(ids))!=len(ids) or not all(0<=i<n for i in ids)):
            raise ValueError("malformed/disallowed subset")
    labels=list(map(int,lines[-1].split()))
    if len(labels)!=n or set(labels)!=set(range(d)):
        raise ValueError("partition labels missing or invalid")
    loads=[0]*d
    for a,w in zip(labels,weights,strict=True):
        loads[a]+=w
    total=sum(loads)
    # Exact integer algebra for variance and comparison with half-integers.
    scaled_sq=10000*(d*sum(x*x for x in loads)-total*total)
    floor=math.isqrt(scaled_sq)//d
    rounded=floor+(4*scaled_sq>=d*d*(2*floor+1)**2)
    return 1+rounded,n,d,q


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    for key in ("inputs", "out", "tester", "baseline", "candidate"):
        parser.add_argument(key, type=Path)
    parser.add_argument("--candidate-label",default="balance")
    parser.add_argument("--rank-only",type=Path)
    a = parser.parse_args()
    cases = sorted(a.inputs.resolve().glob("*.txt"))
    out, tester = a.out.resolve(), a.tester.resolve()
    solvers = [("baseline", a.baseline.resolve()), (a.candidate_label, a.candidate.resolve())]
    if a.rank_only is not None:
        solvers.append(("rank-only",a.rank_only.resolve()))
    out.mkdir(parents=True, exist_ok=True)
    scratch = out/"scratch"
    scratch.mkdir(exist_ok=True)
    manifest = out/"manifest.json"
    meta = {"runner":sha(Path(__file__)), "tester":sha(tester),
            "solvers":{name:sha(path) for name,path in solvers},
            "inputs":{p.name:sha(p) for p in cases}}
    if not cases or manifest.exists() and json.loads(manifest.read_text()) != meta:
        parser.error("no inputs or stale checkpoints")
    manifest.write_text(json.dumps(meta, indent=2)+"\n")
    results = {}
    for i, case in enumerate(cases):
        for name, binary in (solvers if i%2 == 0 else solvers[::-1]):
            p = out/f"{case.stem}-{name}.json"
            if p.exists():
                row = json.loads(p.read_text())
            else:
                start = time.perf_counter()
                run = subprocess.run([tester,binary], input=case.read_bytes(), capture_output=True,
                                     check=True, timeout=2)
                wall = time.perf_counter() - start
                if wall > 2:
                    raise TimeoutError("execution exceeded 2 seconds including tester")
                actions = run.stdout.decode("ascii").strip()
                s, n, d, q = score(case,actions)
                match = re.fullmatch(r"Score = (\d+)\s*",run.stderr.decode("ascii"))
                if match is None or int(match[1]) != s:
                    raise ValueError(f"official disagreement: {case} {s}, {run.stderr!r}")
                row = {"score":s,"wall":wall,"n":n,"d":d,"q":q}
                tmp = p.with_suffix(".tmp")
                tmp.write_text(json.dumps(row)+"\n")
                tmp.rename(p)
            results[name, case.name] = row
    summary = {}
    for name, _ in solvers:
        rows = [results[name,c.name] for c in cases]
        summary[name] = {"valid":len(rows), "total":sum(r["score"] for r in rows),
                          "queries":sum(r["q"] for r in rows),
                          "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"] = sum(results[a.candidate_label,c.name]["score"] <
                                    results["baseline",c.name]["score"] for c in cases)
    if a.rank_only is not None:
        summary["candidate_vs_rank_only_wins"] = sum(results[a.candidate_label,c.name]["score"]<
                                                     results["rank-only",c.name]["score"] for c in cases)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
