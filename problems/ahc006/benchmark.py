"""Paired A/B using disjoint generator seeds and independent output checks."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time


def seeds(split):
    return [int.from_bytes(hashlib.sha256(f"ahc006/spatial-v1/{split}/{i}".encode())
                          .digest()[:8], "little") for i in range(10)]


def check(inp, out):
    data = list(map(int, inp.split()))
    orders = [tuple(data[i:i+4]) for i in range(0, len(data), 4)]
    o = list(map(int, out.split()))
    ids = o[1:51]
    assert o[0] == len(set(ids)) == 50 and all(1 <= i <= 1000 for i in ids)
    points = list(zip(o[52::2], o[53::2], strict=True))
    assert o[51] == len(points) and points[0] == points[-1] == (400, 400)
    first, last = {}, {}
    for j, p in enumerate(points):
        assert all(0 <= x <= 800 for x in p)
        first.setdefault(p, j)
        last[p] = j
    for i in ids:
        a, b, c, d = orders[i-1]
        assert first[(a, b)] < last[(c, d)]
    cost = sum(abs(a-c) + abs(b-d) for (a, b), (c, d) in zip(points, points[1:]))
    k = 1000 + cost
    return cost, (200000000 + k) // (2*k)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ["gen", "vis", "base", "spatial", "csv"]:
        p.add_argument(name, type=Path)
    args = p.parse_args()
    args.gen, args.vis, args.base, args.spatial = (
        x.resolve() for x in (args.gen, args.vis, args.base, args.spatial))
    with tempfile.TemporaryDirectory() as d, args.csv.open("w", newline="") as f:
        work = Path(d)
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["split", "seed", "mode", "cost", "score", "seconds", "initial", "setup_ms", "iterations"])
        for split in ["dev", "holdout"]:
            seed_path = work / "seeds.txt"
            seed_path.write_text("".join(f"{s}\n" for s in seeds(split)))
            subprocess.run([args.gen, seed_path], cwd=work, check=True)
            for i, s in enumerate(seeds(split)):
                inp = work / "in" / f"{i:04}.txt"
                output = work / "out.txt"
                modes = [("base", args.base), ("spatial", args.spatial)]
                for mode, binary in (modes if i % 2 == 0 else modes[::-1]):
                    start = time.monotonic()
                    run = subprocess.run([binary], input=inp.read_bytes(), capture_output=True,
                                         timeout=2.0, check=True)
                    elapsed = time.monotonic() - start
                    assert elapsed < 2.0, (mode, s, elapsed)
                    output.write_bytes(run.stdout)
                    cost, score = check(inp.read_bytes(), run.stdout)
                    vis = subprocess.run([args.vis, inp, output], cwd=work, check=True,
                                         capture_output=True, text=True).stdout.strip()
                    assert vis == f"Score = {score}", (vis, score)
                    stats = dict(re.findall(r"(\w+)=([\d.]+)", run.stderr.decode()))
                    assert int(stats["cost"]) == cost
                    w.writerow([split, s, mode, cost, score, f"{elapsed:.6f}",
                               stats["initial"], stats["setup_ms"], stats["iterations"]])
                    f.flush()


if __name__ == "__main__":
    main()
