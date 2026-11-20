"""Paired scores with the official vis and independent directed-port cycle tracing."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time

S = 30
PORT_PAIRS = (((0, 1),), ((0, 3),), ((2, 3),), ((1, 2),),
              ((0, 1), (2, 3)), ((0, 3), (1, 2)), ((0, 2),), ((1, 3),))
OFFSETS = ((0, -1), (-1, 0), (0, 1), (1, 0))


def independent_score(payload, answer):
    tile = "".join(payload.decode().split())
    rotations = answer.decode().strip()
    assert len(tile) == len(rotations) == S*S
    assert set(rotations) <= set("0123")
    successors = {}
    for i, (t, k) in enumerate(zip(tile, rotations, strict=True)):
        row, col = divmod(i, S)
        for a, b in PORT_PAIRS[int(t)]:
            a, b = (a-int(k)) % 4, (b-int(k)) % 4
            for src, nxt in ((a, b), (b, a)):
                dr, dc = OFFSETS[nxt]
                r, c = row+dr, col+dc
                if 0 <= r < S and 0 <= c < S:
                    successors[i, src] = r*S+c, nxt ^ 2
    done, loops = set(), []
    for v in successors:
        branch = {}
        while v in successors and v not in done and v not in branch:
            branch[v] = len(branch)
            v = successors[v]
        if v in branch:
            loops.append(len(branch)-branch[v])
        done.update(branch)
    loops.sort(reverse=True)
    assert len(loops) % 2 == 0 and all(a == b for a, b in zip(loops[::2], loops[1::2]))
    lengths = (loops[::2] + [0, 0])[:2]
    return lengths[0]*lengths[1], lengths


def seeds(split):
    return [int.from_bytes(hashlib.sha256(f"ahc010/edge-v1/{split}/{i}".encode())
                          .digest()[:8], "little") for i in range(10)]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ("gen", "vis", "base", "variant", "csv"):
        p.add_argument(name, type=Path)
    args = p.parse_args()
    gen, vis, base, variant = (x.resolve() for x in (args.gen, args.vis, args.base, args.variant))
    with tempfile.TemporaryDirectory() as tmp, args.csv.open("w", newline="") as f:
        work = Path(tmp)
        out = work / "out.txt"
        seedfile = work / "seeds.txt"
        writer = csv.writer(f, lineterminator="\n")
        writer.writerow(("split", "seed", "mode", "score", "l1", "l2",
                         "left", "right", "seconds"))
        for split in ("dev", "holdout"):
            seedfile.write_text("".join(f"{s}\n" for s in seeds(split)))
            subprocess.run([gen, seedfile], cwd=work, check=True)
            for i, seed in enumerate(seeds(split)):
                inp = work / "in" / f"{i:04}.txt"
                payload = inp.read_bytes()
                modes = [("base", base), ("edge", variant)]
                for mode, binary in (modes if i % 2 == 0 else modes[::-1]):
                    start = time.monotonic()
                    run = subprocess.run([binary], input=payload, capture_output=True,
                                         check=True, timeout=2)
                    elapsed = time.monotonic()-start
                    assert elapsed < 2
                    score, lengths = independent_score(payload, run.stdout)
                    out.write_bytes(run.stdout)
                    observed = subprocess.run([vis, inp, out], cwd=work, capture_output=True,
                                              check=True, text=True).stdout.strip()
                    assert observed == f"Score = {score}", (mode, seed, observed, score)
                    match = re.fullmatch(rb"left=(\d+) right=(\d+)\s*", run.stderr)
                    assert match
                    left, right = int(match[1]), int(match[2])
                    assert score >= left*right
                    writer.writerow((split, seed, mode, score, *lengths, left, right, f"{elapsed:.6f}"))
                    f.flush()


if __name__ == "__main__":
    main()
