#!/usr/bin/env python3
"""Score one frozen executable on ALE-Bench full private cases.

Only final totals and timings leave a temporary working directory. Download
the AtCoder-provided archive, build gen/vis/tester from its tools with cargo,
and pass the frozen executable and these three tools. Do not use as a
development loop: no private seed lists, inputs, or per-case scores printed.
"""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import zipfile

sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parents[2]))
from benchmark import run

ARCHIVE_SHA = "533ebc1446dd730d4687f973f2ab2300044d606af48b34cb13da9187ff4b5fc1"


def main():
    parser = argparse.ArgumentParser()
    for name in ("archive", "solver", "gen", "vis", "tester", "result"):
        parser.add_argument(name,type=Path)
    a=parser.parse_args()
    archive_sha=hashlib.sha256(a.archive.read_bytes()).hexdigest()
    if archive_sha!=ARCHIVE_SHA:
        parser.error("Unexpected ALE-Bench artifact digest")
    with zipfile.ZipFile(a.archive) as z:
        config=json.loads(z.read("ahc007//data.json"))
        lead=next(csv.DictReader(z.read("ahc007//standings_scores.csv").decode().splitlines()))
    if config["metadata"]["problem_id"]!="ahc007":
        parser.error("Unexpected ALE-Bench problem ID")
    seeds=config["seeds"]["private"]
    if len(seeds)!=150 or len(set(seeds))!=150 or lead!={"rank":"1","score":"14241733941"}:
        parser.error("Unexpected benchmark metadata")
    solver,gen,vis,tester=(getattr(a,key).resolve() for key in ("solver","gen","vis","tester"))
    total=0
    max_wall=0.0
    max_cpu=0.0
    valid=0
    with tempfile.TemporaryDirectory(prefix="ahc007-blind-") as scratch:
        work=Path(scratch)
        (work/"seeds.txt").write_text("\n".join(map(str,seeds))+"\n")
        subprocess.run([gen,"seeds.txt"],cwd=work,check=True,capture_output=True)
        (work/"seeds.txt").unlink()
        cases=sorted((work/"in").glob("*.txt"))
        if len(cases)!=150: raise RuntimeError("Generator count does not match archive")
        for case in cases:
            row=run(case,solver,vis,work)
            with case.open("rb") as inp:
                test=subprocess.run([tester,solver],stdin=inp,check=True,
                                    capture_output=True,cwd=work,timeout=2.0)
            hits=re.findall(rb"^Score = (\d+)\r?$",test.stderr,re.MULTILINE)
            if len(hits)!=1 or int(hits[0])!=row["score"]:
                raise RuntimeError("Tester, visualizer and independent result disagree")
            total+=row["score"]
            max_wall=max(max_wall,row["wall"])
            max_cpu=max(max_cpu,row["cpu"])
            valid+=1
    result={
        "case_count":150,"valid":valid,"official_agreement":valid,
        "original_winner":int(lead["score"]),"total":total,
        "delta":total-int(lead["score"]),"max_lockstep_wall":max_wall,
        "max_solver_cpu":max_cpu,"dataset_sha256":archive_sha,
        "solver_sha256":hashlib.sha256(solver.read_bytes()).hexdigest(),
        "tool_sha256":{str(p.name):hashlib.sha256(p.read_bytes()).hexdigest() for p in (gen,vis,tester)},
    }
    a.result.write_text(json.dumps(result,indent=2)+"\n")
    print(json.dumps(result))


if __name__=="__main__":
    main()
