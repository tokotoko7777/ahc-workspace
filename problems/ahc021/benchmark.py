#!/usr/bin/env python3
"""Replay swaps independently and compare the resulting pyramid score with official vis."""
import argparse
import hashlib
import json
import re
import subprocess
import time
from pathlib import Path


def score(case,answer):
    nums=list(map(int,case.read_text().split()))
    n=30
    board=[nums[r*(r+1)//2:(r+1)*(r+2)//2] for r in range(n)]
    k,*coords=map(int,answer.split())
    if not 0<=k<=10000 or len(coords)!=4*k:
        raise ValueError("bad swap count")
    legal={(0,-1),(0,1),(1,0),(1,1),(-1,0),(-1,-1)}
    for r,c,y,x in zip(coords[0::4],coords[1::4],coords[2::4],coords[3::4],strict=True):
        if not (0<=c<=r<n and 0<=x<=y<n and (y-r,x-c) in legal):
            raise ValueError("invalid swap")
        board[r][c],board[y][x]=board[y][x],board[r][c]
    errors=sum(board[r][c]>board[r+1][c+d]
               for r in range(n-1) for c in range(r+1) for d in (0,1))
    return (50000-50*errors if errors else 100000-5*k), k, errors


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    for key in ("inputs", "out", "vis", "baseline", "two_ended"):
        parser.add_argument(key, type=Path)
    a = parser.parse_args()
    cases = sorted(a.inputs.resolve().glob("*.txt"))
    out, vis = a.out.resolve(), a.vis.resolve()
    solvers = [("baseline", a.baseline.resolve()), ("two-ended", a.two_ended.resolve())]
    out.mkdir(parents=True, exist_ok=True)
    scratch = out/"scratch"
    scratch.mkdir(exist_ok=True)
    manifest = out/"manifest.json"
    meta = {"runner":sha(Path(__file__)), "vis":sha(vis),
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
                run = subprocess.run([binary], input=case.read_bytes(), capture_output=True,
                                     check=True, timeout=2)
                wall = time.perf_counter() - start
                if wall > 2:
                    raise TimeoutError("execution exceeded 2 seconds")
                actions = run.stdout.decode("ascii").strip()
                output = scratch/"output.txt"
                output.write_text(actions+"\n")
                s, swaps, errors = score(case,actions)
                actual = subprocess.run([vis, case, output], cwd=scratch, capture_output=True,
                                        text=True, check=True, timeout=90)
                match = re.fullmatch(r"Score = (\d+)\s*", actual.stdout)
                if match is None or int(match[1]) != s:
                    raise ValueError(f"official disagreement: {case} {s}, {actual.stdout!r}")
                row = {"score":s, "wall":wall, "swaps":swaps, "errors":errors}
                tmp = p.with_suffix(".tmp")
                tmp.write_text(json.dumps(row)+"\n")
                tmp.rename(p)
            results[name, case.name] = row
    summary = {}
    for name, _ in solvers:
        rows = [results[name,c.name] for c in cases]
        summary[name] = {"valid":len(rows), "total":sum(r["score"] for r in rows),
                          "complete":sum(r["errors"]==0 for r in rows),
                          "swaps":sum(r["swaps"] for r in rows),
                          "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"] = sum(results["two-ended",c.name]["score"] >
                                    results["baseline",c.name]["score"] for c in cases)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
