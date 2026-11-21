"""Quick sequential development evaluation using freshly generated official cases.

The full benchmark_cases.py additionally replays recorded actions and is used for
checkpoint verification. This script permits command arguments for experiments.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import sys
import time

def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("generator", type=Path)
    parser.add_argument("tester", type=Path)
    parser.add_argument("directory", type=Path)
    parser.add_argument("split")
    parser.add_argument("count", type=int)
    parser.add_argument("solver", type=Path)
    parser.add_argument("solver_args", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    directory = args.directory.resolve()
    directory.mkdir(parents=True, exist_ok=True)
    split = args.split
    keys = [
        int.from_bytes(hashlib.sha256(f"ahc008/habitat/1/{split}/{i}".encode()).digest()[:8], "little")
        for i in range(args.count)
    ]
    seedfile = directory / "seeds.txt"
    seedfile.write_text("".join(f"{k}\n" for k in keys))
    subprocess.run([args.generator.resolve(), seedfile.name], cwd=directory, check=True)

    writer = csv.writer(sys.stdout, lineterminator="\n")
    writer.writerow(["case", "score", "tester_wall_seconds"])
    for i in range(args.count):
        case = (directory / "in" / f"{i:04}.txt").read_bytes()
        before = time.perf_counter()
        proc = subprocess.run(
            [args.tester.resolve(), args.solver.resolve(), *args.solver_args],
            input=case, capture_output=True, check=True, timeout=3.0,
        )
        elapsed = time.perf_counter() - before
        match = re.fullmatch(rb"Score = ([1-9]\d*)\s*", proc.stderr)
        if not match:
            raise RuntimeError(f"{i}: tester did not return a legal positive score ({proc.stderr!r})")
        writer.writerow([i, int(match[1]), f"{elapsed:.6f}"])
        sys.stdout.flush()

if __name__ == "__main__":
    main()
