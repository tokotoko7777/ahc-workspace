"""Measure named GNU C++23 solvers on the same fixed set of official-format cases."""

import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import re
import signal
import statistics
import subprocess
import sys
import tempfile
import time

from benchmark import replay


def run(command, case, timeout):
    start = time.monotonic()
    with subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE, start_new_session=True) as proc:
        try:
            out, err = proc.communicate(case, timeout=timeout)
            return out, err, proc.returncode, time.monotonic() - start
        except subprocess.TimeoutExpired:
            os.killpg(proc.pid, signal.SIGKILL)
            proc.communicate()
            return b"", b"", None, time.monotonic() - start


def playback(actions_file, trace_file):
    """Regenerate the pet replies for recorded actions without rerunning solver."""
    source = sys.stdin.buffer
    n = int(source.readline())
    for _ in range(n):
        assert source.readline()
    humans = int(source.readline())
    for _ in range(humans):
        assert source.readline()
    actions = actions_file.read_bytes().splitlines()
    with trace_file.open("wb") as log:
        # Comments, if any, are preserved in tester stdout but are not turns.
        num = 0
        for action in actions:
            sys.stdout.buffer.write(action + b"\n")
            sys.stdout.buffer.flush()
            if action.startswith(b"#"):
                continue
            reply = source.readline()
            assert reply, "tester ended before 300 commands"
            log.write(action + b"\n" + reply)
            num += 1
        assert num == 300


def evaluate(tester, binary, case, trace, timeout, snapshots=None, interval=50):
    out, err, rc, wall = run([tester, binary], case, timeout)
    match = re.fullmatch(rb"Score = (\d+)\s*", err)
    if rc is None:
        return 0, wall, [], 0, 0, "", "timeout"
    if rc or not match:
        return 0, wall, [], 0, 0, "", f"tester error {rc}: {err[:200]!r}"
    trace.unlink(missing_ok=True)
    actions = trace.with_suffix(".actions")
    actions.write_bytes(out)
    script = Path(__file__).resolve()
    check_out, check_err, check_rc, _ = run(
        [tester, sys.executable, script, "--playback", actions, trace], case, max(10, timeout))
    if check_rc != 0 or check_out != out or check_err != err:
        raise RuntimeError(f"recorded-action playback disagrees for {binary}: {check_rc}, {check_err[:300]!r}")
    commands = [x for x in out.decode().splitlines() if not x.startswith("#")]
    score, humans, walls, largest = replay(case.decode(), trace.read_text().splitlines(),
                                           commands, topology=True, snapshots=snapshots, interval=interval)
    if score != int(match[1]):
        raise RuntimeError(f"independent score {score} differs from tester {match[1]!r}")
    return score, wall, humans, walls, largest, hashlib.sha256(out).hexdigest(), "ok"


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("tester", type=Path)
    p.add_argument("cases", type=Path)
    p.add_argument("csv", type=Path)
    p.add_argument("solver", nargs="+", help="one or more LABEL=PATH arguments")
    p.add_argument("--count", type=int, default=100)
    p.add_argument("--timeout", type=float, default=3.0)
    p.add_argument("--telemetry", type=Path)
    p.add_argument("--interval", type=int, default=50)
    a = p.parse_args()
    if a.interval <= 0 or a.count <= 0:
        p.error("count and interval must be positive")
    models = {}
    for entry in a.solver:
        label, sep, value = entry.partition("=")
        if not sep or not label or not value or label in models:
            p.error(f"expected unique LABEL=PATH: {entry}")
        models[label] = Path(value).resolve(strict=True)
    cases = sorted(a.cases.glob("*.txt"))
    if len(cases) != a.count:
        p.error(f"expected {a.count} case files, got {len(cases)}")
    telemetry = []
    data = {k: [] for k in models}
    digest = hashlib.sha256()
    with tempfile.TemporaryDirectory() as td, a.csv.open("x", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(("case", "model", "score", "wall_seconds", "status", "walls", "all_humans_pet_free",
                    "pet_free_humans", "humans", "human_areas", "human_reachable_pets",
                    "largest_component_area", "fixed_human_cap", "action_sha256"))
        for i, case_path in enumerate(cases):
            case = case_path.read_bytes()
            digest.update(case_path.name.encode() + b"\0" + hashlib.sha256(case).digest())
            labels = list(models)
            labels = labels[i % len(labels):] + labels[:i % len(labels)]
            for label in labels:
                def collect(turn, groups):
                    telemetry.append({"case": case_path.name, "model": label, "turn": turn,
                                      "groups": groups})
                score, wall, people, walls, largest, action_sha, status = evaluate(a.tester.resolve(), models[label],
                    case, Path(td) / "relay.txt", a.timeout,
                    snapshots=collect if a.telemetry else None, interval=a.interval)
                free = sum(k == 0 for _, k in people)
                all_free = bool(people) and free == len(people)
                h = len(people)
                fixed_cap = ((10**8*sum(x for x,_ in people) + 900*h - 1) // (900*h)) if h else 0
                w.writerow((case_path.name, label, score, f"{wall:.6f}", status, walls,
                    int(all_free), free, len(people), " ".join(str(x) for x, _ in people),
                    " ".join(str(x) for _, x in people), largest, fixed_cap, action_sha))
                data[label].append((score, wall, status, int(all_free), people,
                    (10**8*largest + 899) // 900, fixed_cap))
            f.flush()
    if a.telemetry:
        with a.telemetry.open("x") as tf:
            for row in telemetry:
                tf.write(json.dumps(row, separators=(",", ":"))+"\n")
    report = {"case_count": len(cases), "case_set_sha256": digest.hexdigest(), "models": {}}
    base = next(iter(models))
    for label, rows in data.items():
        diff = [x[0] - y[0] for x, y in zip(rows, data[base], strict=True)]
        people = [p for row in rows for p in row[4]]
        free_areas = [area for area, pets in people if pets == 0]
        report["models"][label] = {
            "solver_sha256": hashlib.sha256(models[label].read_bytes()).hexdigest(),
            "score_sum": sum(x[0] for x in rows), "failed": sum(x[2] != "ok" for x in rows),
            "max_wall": max(x[1] for x in rows), "median_wall": statistics.median(x[1] for x in rows),
            "pet_free_cases": sum(x[3] for x in rows), "pet_free_humans": len(free_areas),
            "observed_humans": len(people), "mean_pet_free_area": statistics.mean(free_areas) if free_areas else None,
            "mean_human_area": statistics.mean(a for a, _ in people) if people else None,
            "mean_reachable_pets": statistics.mean(k for _, k in people) if people else None,
            "perfect_pet_exclusion_cap_with_fixed_humans": sum(x[6] for x in rows),
            "perfect_pet_exclusion_and_placement_cap": sum(x[5] for x in rows),
            "wins_vs_first": sum(v > 0 for v in diff), "losses_vs_first": sum(v < 0 for v in diff),
        }
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    if len(sys.argv) == 4 and sys.argv[1] == "--playback":
        playback(Path(sys.argv[2]), Path(sys.argv[3]))
    else:
        main()
