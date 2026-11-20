"""Matched frozen-start diagnostic, using new official-generator seeds."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time

from benchmark import check


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("split", choices=("dev", "holdout"))
    for name in ("gen", "vis", "diagnostic", "csv"):
        p.add_argument(name, type=Path)
    args = p.parse_args()
    gen, vis, diagnostic = [x.resolve() for x in (args.gen, args.vis, args.diagnostic)]
    names = ("frozen", "greedy", "beam", "trials", "greedy_improved", "beam_improved")
    seeds = [int.from_bytes(hashlib.sha256(
        f"ahc006/batch-repair-v1/{args.split}/{i}".encode()).digest()[:8], "little")
        for i in range(8)]
    with tempfile.TemporaryDirectory() as d, args.csv.open("w", newline="") as f:
        work = Path(d)
        w = csv.writer(f, lineterminator="\n")
        w.writerow(("split", "seed", *names, "beam_score", "seconds"))
        seed_path = work / "seeds.txt"
        seed_path.write_text("".join(f"{s}\n" for s in seeds))
        subprocess.run([gen, seed_path], cwd=work, check=True)
        output = work / "out.txt"
        for i, seed in enumerate(seeds):
            inp = work / "in" / f"{i:04}.txt"
            payload = inp.read_bytes()
            start = time.monotonic()
            run = subprocess.run([diagnostic], input=payload, timeout=2,
                                 capture_output=True, check=True)
            sec = time.monotonic() - start
            assert sec < 2
            values = dict(re.findall(r"(\w+)=(\d+)", run.stderr.decode()))
            cost, score = check(payload, run.stdout)
            assert cost == int(values["beam"])
            output.write_bytes(run.stdout)
            observed = subprocess.run([vis, inp, output], cwd=work, check=True,
                                      capture_output=True, text=True).stdout.strip()
            assert observed == f"Score = {score}", (observed, score)
            w.writerow((args.split, seed, *(values[n] for n in names), score, f"{sec:.6f}"))
            f.flush()


if __name__ == "__main__":
    main()
