#!/usr/bin/env python3
"""Verify capacity and raw complaint independently, and compare with official vis."""
import argparse
import hashlib
import json
import re
import subprocess
import time
from pathlib import Path


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    for key in ("inputs", "out", "vis", "oracle", "baseline", "backup"):
        parser.add_argument(key, type=Path)
    a = parser.parse_args()
    cases = sorted(a.inputs.resolve().glob("*.txt"))
    out, vis = a.out.resolve(), a.vis.resolve()
    solvers = [("baseline", a.baseline.resolve()), ("backup", a.backup.resolve())]
    out.mkdir(parents=True, exist_ok=True)
    oracle = a.oracle.resolve()
    scratch = out/"scratch"
    scratch.mkdir(exist_ok=True)
    manifest = out/"manifest.json"
    meta = {"runner":sha(Path(__file__)), "vis":sha(vis), "oracle":sha(oracle),
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
                independent = subprocess.run([oracle,case,output], capture_output=True,
                                             text=True,check=True,timeout=90)
                s, disconnected = map(int,independent.stdout.split())
                actual = subprocess.run([vis, case, output], cwd=scratch, capture_output=True,
                                        text=True, check=True, timeout=90)
                match = re.fullmatch(r"Score = (\d+)\s*", actual.stdout)
                if match is None or int(match[1]) != s:
                    raise ValueError(f"official disagreement: {case} {s}, {actual.stdout!r}")
                row = {"score":s, "wall":wall, "disconnected":disconnected}
                tmp = p.with_suffix(".tmp")
                tmp.write_text(json.dumps(row)+"\n")
                tmp.rename(p)
            results[name, case.name] = row
    summary = {}
    for name, _ in solvers:
        rows = [results[name,c.name] for c in cases]
        summary[name] = {"valid":len(rows), "total":sum(r["score"] for r in rows),
                          "disconnected_days":sum(r["disconnected"] for r in rows),
                          "max_wall":max(r["wall"] for r in rows)}
    summary["candidate_wins"] = sum(results["backup",c.name]["score"] <
                                    results["baseline",c.name]["score"] for c in cases)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
