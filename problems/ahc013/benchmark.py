#!/usr/bin/env python3
"""Replay moves and cables, then check the resulting clusters against official vis."""
import argparse
import hashlib
import json
import re
import subprocess
import time
from pathlib import Path


def score(case, answer):
    head, *rows = case.read_text().splitlines()
    n, types = map(int, head.split())
    grid = [[int(c) for c in row] for row in rows]
    cells = {(r,c) for r in range(n) for c in range(n)}
    tokens = iter(map(int, answer.split()))
    def pair():
        a, b = (next(tokens),next(tokens)),(next(tokens),next(tokens))
        if a not in cells or b not in cells or a == b:
            raise ValueError("invalid endpoints")
        return a,b
    m = next(tokens)
    if m < 0 or m > 100*types:
        raise ValueError("bad move count")
    for _ in range(m):
        (a,b),(c,d) = pair()
        if abs(a-c)+abs(b-d) != 1 or grid[a][b] == 0 or grid[c][d] != 0:
            raise ValueError("invalid move")
        grid[a][b],grid[c][d] = grid[c][d],grid[a][b]
    k = next(tokens)
    if k < 0 or k+m > 100*types:
        raise ValueError("bad cable count")
    wires = set()
    links = {}
    for _ in range(k):
        (a,b),(c,d) = pair()
        if (a!=c and b!=d) or not grid[a][b] or not grid[c][d] or (c,d) in links.get((a,b), ()):
            raise ValueError("illegal cable endpoints")
        start,end = sorted(((a,b),(c,d)))
        dr,dc = int(a!=c),int(b!=d)
        interior = (start[0]+dr,start[1]+dc)
        while interior != end:
            y,x = interior
            if grid[y][x] or interior in wires:
                raise ValueError("blocked or intersecting cable")
            wires.add(interior)
            interior = y+dr,x+dc
        links.setdefault((a,b), []).append((c,d))
        links.setdefault((c,d), []).append((a,b))
    if next(tokens, None) is not None:
        raise ValueError("trailing output")
    seen = set()
    total = 0
    for start in links:
        if start in seen:
            continue
        stack, counts = [start], [0]*(types+1)
        seen.add(start)
        while stack:
            y,x = stack.pop()
            counts[grid[y][x]]+=1
            for j in links[y,x]:
                if j not in seen:
                    seen.add(j)
                    stack.append(j)
        total += sum(c*(c-1) for c in counts) - sum(counts)*(sum(counts)-1)//2
    return max(0,total), n,types,m,k

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    for key in ("inputs", "out", "vis", "baseline", "motion"):
        parser.add_argument(key, type=Path)
    a = parser.parse_args()
    cases = sorted(a.inputs.resolve().glob("*.txt"))
    out, vis = a.out.resolve(), a.vis.resolve()
    solvers = [("baseline", a.baseline.resolve()), ("motion", a.motion.resolve())]
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
                                     check=True, timeout=3)
                wall = time.perf_counter() - start
                if wall > 3:
                    raise TimeoutError("execution exceeded 3 seconds")
                actions = run.stdout.decode("ascii").strip()
                s, n, k, moves, cables = score(case, actions)
                output = scratch/"output.txt"
                output.write_text(actions+"\n")
                actual = subprocess.run([vis, case, output], cwd=scratch, capture_output=True,
                                        text=True, check=True, timeout=10)
                match = re.fullmatch(r"Score = (\d+)\s*", actual.stdout)
                if match is None or int(match[1]) != s:
                    raise ValueError(f"official disagreement: {case} {s}, {actual.stdout!r}")
                row = {"score":s, "wall":wall, "n":n, "k":k, "moves":moves, "cables":cables}
                tmp = p.with_suffix(".tmp")
                tmp.write_text(json.dumps(row)+"\n")
                tmp.rename(p)
            results[name, case.name] = row
    summary = {}
    for name, _ in solvers:
        rows = [results[name,c.name] for c in cases]
        summary[name] = {"valid":len(rows), "total":sum(r["score"] for r in rows),
                          "moves":sum(r["moves"] for r in rows),
                          "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"] = sum(results["motion",c.name]["score"] >
                                    results["baseline",c.name]["score"] for c in cases)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
