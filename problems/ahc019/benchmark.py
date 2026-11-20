#!/usr/bin/env python3
"""Verify silhouettes, connectivity and proper-rotation alignment, then compare with official vis."""
import argparse
import hashlib
import itertools
from collections import defaultdict
import json
import re
import subprocess
import time
from pathlib import Path


AXES=(0,1,2)
ROTATIONS=[]
for p in itertools.permutations(AXES):
    parity=(-1)**sum(p[a]>p[b] for a,b in itertools.combinations(AXES,2))
    for signs in itertools.product((-1,1),repeat=3):
        if parity*signs[0]*signs[1]*signs[2]==1:
            ROTATIONS.append((p,signs))


def translation_key(points):
    origin=tuple(min(p[i] for p in points) for i in AXES)
    return {tuple(p[i]-origin[i] for i in AXES) for p in points}


def related(a,b):
    if len(a)!=len(b):
        return False
    goal=translation_key(a)
    for perm,sign in ROTATIONS:
        rotated=[tuple(sign[i]*v[perm[i]] for i in AXES) for v in b]
        if translation_key(rotated)==goal:
            return True
    return False


def score(case, answer):
    data=case.read_text().split()
    d=int(data[0])
    labels,*ids=map(int,answer.split())
    if labels<1 or len(ids)!=2*d**3 or any(not 0<=v<=labels for v in ids):
        raise ValueError("invalid labels or grid size")
    points=[defaultdict(set),defaultdict(set)]
    for s in range(2):
        expected_front, expected_right=(
            {(z,x) for z in range(d) for x in range(d) if data[1+(2*s+view)*d+z][x]=='1'}
            for view in range(2))
        front, right=set(),set()
        for j,v in enumerate(ids[s*d**3:(s+1)*d**3]):
            if not v:
                continue
            x,y,z=j//(d*d),j//d%d,j%d
            front.add((z,x)); right.add((z,y))
            points[s][v].add((x,y,z))
        if front != expected_front or right != expected_right:
            raise ValueError("projection differs")
        for label,voxels in points[s].items():
            a=next(iter(voxels))
            reached={a}; stack=[a]
            while stack:
                p=stack.pop()
                for i in AXES:
                    for step in (-1,1):
                        q=list(p); q[i]+=step; q=tuple(q)
                        if q in voxels and q not in reached:
                            reached.add(q); stack.append(q)
            if len(reached)!=len(voxels):
                raise ValueError(f"label {label} not connected")
    value=0.0
    shared=largest=single_volume=0
    for label in range(1,labels+1):
        a,b=points[0][label],points[1][label]
        if not a and not b:
            raise ValueError("unused label")
        if a and b:
            if not related(a,b):
                raise ValueError(f"label {label} not related by a proper rotation")
            shared+=1; largest=max(largest,len(a)); value+=1.0/len(a)
        else:
            single_volume+=len(a)+len(b); value+=len(a)+len(b)
    return int(value*1e9+0.5), largest, single_volume, shared


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    for key in ("inputs", "out", "vis", "baseline", "blocks"):
        parser.add_argument(key, type=Path)
    a = parser.parse_args()
    cases = sorted(a.inputs.resolve().glob("*.txt"))
    out, vis = a.out.resolve(), a.vis.resolve()
    solvers = [("baseline", a.baseline.resolve()), ("blocks", a.blocks.resolve())]
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
                                     check=True, timeout=6)
                wall = time.perf_counter() - start
                if wall > 6:
                    raise TimeoutError("execution exceeded 6 seconds")
                actions = run.stdout.decode("ascii").strip()
                output = scratch/"output.txt"
                output.write_text(actions+"\n")
                s, largest, unmatched, shared = score(case,actions)
                actual = subprocess.run([vis, case, output], cwd=scratch, capture_output=True,
                                        text=True, check=True, timeout=90)
                match = re.fullmatch(r"Score = (\d+)\s*", actual.stdout)
                if match is None or int(match[1]) != s:
                    raise ValueError(f"official disagreement: {case} {s}, {actual.stdout!r}")
                row = {"score":s, "wall":wall, "largest":largest, "unmatched":unmatched,"shared":shared}
                tmp = p.with_suffix(".tmp")
                tmp.write_text(json.dumps(row)+"\n")
                tmp.rename(p)
            results[name, case.name] = row
    summary = {}
    for name, _ in solvers:
        rows = [results[name,c.name] for c in cases]
        summary[name] = {"valid":len(rows), "total":sum(r["score"] for r in rows),
                          "largest_shared":max(r["largest"] for r in rows),
                          "unmatched":sum(r["unmatched"] for r in rows),
                          "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"] = sum(results["blocks",c.name]["score"] <
                                    results["baseline",c.name]["score"] for c in cases)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
