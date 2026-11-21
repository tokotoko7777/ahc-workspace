#!/usr/bin/env python3
"""Drive the solver one edge at a time, then check the official visualizer score."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import resource
import select
import subprocess
import time

N, M = 400, 1995


class Dsu:
    def __init__(self):
        self.p = list(range(N))
        self.parts = N

    def find(self, x):
        while self.p[x] != x:
            self.p[x] = self.p[self.p[x]]
            x = self.p[x]
        return x

    def union(self, a, b):
        a, b = self.find(a), self.find(b)
        if a == b:
            return False
        self.p[a] = b
        self.parts -= 1
        return True


def independent_score(edge_lines, lines, answers):
    full, accepted = Dsu(), Dsu()
    edges = [tuple(map(int, edge.split())) for edge in edge_lines]
    a, b = 0, 0
    items = [(int(w), u, v) for (u, v), w in zip(edges, lines, strict=True)]
    for (w, u, v), take in zip(items, answers, strict=True):
        if take == b"1\n":
            if not accepted.union(u, v):
                raise ValueError("solver returned a cycle")
            a += w
    for w, u, v in sorted(items):
        if full.union(u, v):
            b += w
    if full.parts != 1 or accepted.parts != 1 or a <= 0:
        raise ValueError("disconnected or zero-length result")
    q, r = divmod(10**8 * b, a)
    return q + (2*r >= a)


def cpu_used():
    ru = resource.getrusage(resource.RUSAGE_CHILDREN)
    return ru.ru_utime + ru.ru_stime


def run(case, binary, vis, out):
    lines = case.read_bytes().splitlines(keepends=True)
    if len(lines) != N+2*M:
        raise ValueError(f"unexpected generated input size: {case}")
    answers = []
    cpu0 = cpu_used()
    start = time.perf_counter()
    with subprocess.Popen([binary], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, bufsize=0) as proc:
        try:
            def send(data):
                pending = memoryview(data)
                while pending:
                    count = os.write(proc.stdin.fileno(), pending)
                    pending = pending[count:]

            # A one-way preload would not detect accidentally reading ahead.
            send(b"".join(lines[:N+M]))
            pending = b""
            for w in lines[N+M:]:
                send(w.rstrip() + b"\n")
                while b"\n" not in pending:
                    remaining = 2.0 - (time.perf_counter()-start)
                    if remaining <= 0 or not select.select([proc.stdout], [], [], remaining)[0]:
                        raise TimeoutError("2s lock-step deadline")
                    chunk = os.read(proc.stdout.fileno(), 64)
                    if not chunk:
                        raise ValueError("premature EOF")
                    pending += chunk
                    if len(pending) > 2:
                        raise ValueError("extra or malformed output")
                ans, pending = pending.split(b"\n", 1)
                if ans not in (b"0", b"1"):
                    raise ValueError("expected a decision bit")
                answers.append(ans+b"\n")
            proc.stdin.close()
            exit_code = proc.wait(timeout=max(0.001, 2.0-(time.perf_counter()-start)))
            diagnostics=proc.stderr.read().decode("ascii",errors="replace")
            if exit_code or pending or proc.stdout.read(1):
                raise ValueError("bad exit or trailing output")
        except Exception:
            proc.kill()
            proc.wait()
            raise
    wall = time.perf_counter()-start
    cpu = cpu_used()-cpu0
    if wall > 2.0 or cpu > 2.0:
        raise TimeoutError("runtime limit")
    score = independent_score(lines[N:N+M], lines[N+M:], answers)
    output = out/"decisions.txt"
    output.write_bytes(b"".join(answers))
    scored = subprocess.run([vis, case, output], cwd=out, capture_output=True,
                            text=True, check=True, timeout=10)
    match = re.fullmatch(r"Score = (\d+)\s*", scored.stdout)
    if match is None or int(match[1]) != score:
        raise ValueError(f"official disagreement: {score}, {scored.stdout!r}")
    return {"score":score, "wall":wall, "cpu":cpu,
            "decisions":hashlib.sha256(b"".join(answers)).hexdigest(),
            "diagnostics":{k:int(v) for k,v in re.findall(r"(\w+)=(\d+)",diagnostics)}}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    for key in ("inputs", "out", "vis", "baseline", "scenario"):
        parser.add_argument(key, type=Path)
    a = parser.parse_args()
    cases = sorted(a.inputs.resolve().glob("*.txt"))
    out, vis = a.out.resolve(), a.vis.resolve()
    solvers = [("baseline", a.baseline.resolve()), ("scenario",a.scenario.resolve())]
    out.mkdir(parents=True, exist_ok=True)
    (out/"scratch").mkdir(exist_ok=True)
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
                row = run(case, binary, vis, out/"scratch")
                tmp = p.with_suffix(".tmp")
                tmp.write_text(json.dumps(row)+"\n")
                tmp.rename(p)
            results[name, case.name] = row
    summary = {}
    for name, _ in solvers:
        rows = [results[name,c.name] for c in cases]
        summary[name] = { "valid":len(rows), "total":sum(r["score"] for r in rows),
                          "max_wall":max(r["wall"] for r in rows),
                          "max_cpu":max(r["cpu"] for r in rows),
                          "diagnostics":{k:sum(r["diagnostics"].get(k,0) for r in rows)
                               for k in sorted({x for r in rows for x in r["diagnostics"]})} }
    summary["candidate_wins"] = sum(results["scenario",c.name]["score"] >
                                    results["baseline",c.name]["score"] for c in cases)
    summary["identical_decisions"]=sum(results["scenario",c.name]["decisions"] ==
                                           results["baseline",c.name]["decisions"] for c in cases)
    (out/"summary.json").write_text(json.dumps(summary,indent=2)+"\n")
    print(json.dumps(summary))


if __name__ == "__main__":
    main()
