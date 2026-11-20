#!/usr/bin/env python3
"""Official lock-step tester and independent integer card/cash replay."""
import argparse
import hashlib
import json
import re
import subprocess
import time
from pathlib import Path


def score(case,transcript,level_turns=None):
    tokens=iter(map(int,case.read_text().split()))
    n,m,k,t=(next(tokens) for _ in range(4))
    projects=[[next(tokens),next(tokens)] for _ in range(m)]
    replacement=[(next(tokens),next(tokens)) for _ in range(m*t)]
    hand=[(next(tokens),next(tokens)) for _ in range(n)]
    offers=[[(next(tokens),next(tokens),next(tokens)) for _ in range(k)] for _ in range(t)]
    if next(tokens,None) is not None:
        raise ValueError("trailing generator data")
    moves=[list(map(int,line.split())) for line in transcript.splitlines()]
    if len(moves)!=2*t:
        raise ValueError("incorrect number of decisions")
    level=money=pos=0
    def replace(i):
        nonlocal pos
        h,v=replacement[pos];pos+=1
        projects[i]=[h<<level,v<<level]
    for day in range(t):
        (c,j),(a,)=moves[2*day:2*day+2]
        if not (0<=c<n and 0<=j<m and 0<=a<k):
            raise ValueError("decision index out of range")
        typ,work=hand[c]
        if typ in (1,3,4) and j:
            raise ValueError("global card requires target zero")
        if typ==4:
            level+=1
            if level_turns is not None:
                level_turns.append(day+1)
            if level>20:
                raise ValueError("capital increase cap")
        else:
            affected=(j,) if typ in (0,2) else range(m)
            for i in affected:
                if typ in (2,3):
                    replace(i)
                else:
                    projects[i][0]-=work
                    if projects[i][0]<=0:
                        money+=projects[i][1]
                        replace(i)
        typ,w,p=offers[day][a]
        if p<<level>money:
            raise ValueError("unaffordable purchase")
        money-=p<<level
        hand[c]=(typ,w<<level)
    return money,level,m,k

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    for key in ("inputs", "out", "tester", "baseline", "candidate"):
        parser.add_argument(key, type=Path)
    parser.add_argument("--candidate-label",default="forecast")
    parser.add_argument("--rank-only",type=Path)
    parser.add_argument("--secondary",type=Path)
    parser.add_argument("--secondary-label",default="secondary")
    a = parser.parse_args()
    cases = sorted(a.inputs.resolve().glob("*.txt"))
    out, tester = a.out.resolve(), a.tester.resolve()
    solvers = [("baseline", a.baseline.resolve()), (a.candidate_label, a.candidate.resolve())]
    if a.secondary is not None:
        solvers.append((a.secondary_label,a.secondary.resolve()))
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
                level_turns = []
                s, level, m, k = score(case,actions,level_turns)
                match = re.fullmatch(r"Score = (\d+)\s*",run.stderr.decode("ascii"))
                if match is None or int(match[1]) != s:
                    raise ValueError(f"official disagreement: {case} {s}, {run.stderr!r}")
                row = {"score":s,"wall":wall,"level":level,"m":m,"k":k,"level_turns":level_turns}
                tmp = p.with_suffix(".tmp")
                tmp.write_text(json.dumps(row)+"\n")
                tmp.rename(p)
            results[name, case.name] = row
    summary = {}
    for name, _ in solvers:
        rows = [results[name,c.name] for c in cases]
        summary[name] = {"valid":len(rows), "total":sum(r["score"] for r in rows),
                          "investments":sum(r["level"] for r in rows),
                          "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"] = sum(results[a.candidate_label,c.name]["score"] >
                                    results["baseline",c.name]["score"] for c in cases)
    if a.secondary is not None:
        summary["secondary_wins"] = sum(results[a.secondary_label,c.name]["score"] >
                                        results["baseline",c.name]["score"] for c in cases)
    if a.rank_only is not None:
        summary["candidate_vs_rank_only_wins"] = sum(results[a.candidate_label,c.name]["score"]<
                                                     results["rank-only",c.name]["score"] for c in cases)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
