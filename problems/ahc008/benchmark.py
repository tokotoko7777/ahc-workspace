"""Run official tester against both arms and replay its live transcript."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import time

STEPS = {"U": (-1, 0), "D": (1, 0), "L": (0, -1), "R": (0, 1)}


def relay(binary, trace):
    """Transparent protocol proxy; the official tester controls animal RNG."""
    with subprocess.Popen([binary], stdin=subprocess.PIPE, stdout=subprocess.PIPE) as child, \
            trace.open("wb") as log:
        nline = sys.stdin.buffer.readline()
        initial = nline + b"".join(sys.stdin.buffer.readline() for _ in range(int(nline)))
        mline = sys.stdin.buffer.readline()
        initial += mline + b"".join(sys.stdin.buffer.readline() for _ in range(int(mline)))
        child.stdin.write(initial)
        child.stdin.flush()
        for _ in range(300):
            action = child.stdout.readline()
            assert action, "solver terminated early"
            sys.stdout.buffer.write(action)
            sys.stdout.buffer.flush()
            reply = sys.stdin.buffer.readline()
            assert reply, "tester rejected output"
            child.stdin.write(reply)
            child.stdin.flush()
            log.write(action + reply)
        child.stdin.close()
        assert not child.stdout.read(), "trailing output after turn 300"
        assert child.wait() == 0


def add(p, step):
    r, c = STEPS[step]
    return p[0] + r, p[1] + c


def inside(p):
    return 1 <= p[0] <= 30 and 1 <= p[1] <= 30


def replay(case, trace, commands):
    f = iter(case.splitlines())
    pets = [tuple(map(int, next(f).split()[:2])) for _ in range(int(next(f)))]
    humans = [tuple(map(int, next(f).split())) for _ in range(int(next(f)))]
    assert len(trace) == 600 and trace[::2] == commands
    blocked = set()
    for cmd, reply in zip(trace[::2], trace[1::2], strict=True):
        assert len(cmd) == len(humans)
        forbidden = set(humans + pets)
        forbidden |= {add(p, s) for p in pets for s in STEPS}
        newly = set()
        after = []
        for p, op in zip(humans, cmd, strict=True):
            if op == ".":
                after.append(p)
                continue
            q = add(p, op.upper())
            assert inside(q)
            if op.islower():
                assert q not in forbidden
                newly.add(q)
                after.append(p)
            else:
                after.append(q)
        blocked |= newly
        assert all(p not in blocked for p in after)
        humans = after
        for i, steps in enumerate(reply.split()):
            for op in steps:
                if op != ".":
                    pets[i] = add(pets[i], op)
                    assert inside(pets[i]) and pets[i] not in blocked
        assert len(reply.split()) == len(pets)
    values = []
    for h in humans:
        todo = [h]
        seen = {h}
        while todo:
            p = todo.pop()
            for s in STEPS:
                q = add(p, s)
                if inside(q) and q not in seen and q not in blocked:
                    seen.add(q)
                    todo.append(q)
        values.append((len(seen), sum(p in seen for p in pets)))
    n = len(pets)
    numerator = 10**8 * sum(a << (n-k) for a, k in values)
    denominator = (900 * len(humans)) << n
    score = (2 * numerator + denominator) // (2 * denominator)
    return score, values[0][0], values[0][1], len(blocked)


def seeds(split):
    return [int.from_bytes(hashlib.sha256(f"ahc008/cuts-v1/{split}/{i}".encode())
                          .digest()[:8], "little") for i in range(24)]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ("gen", "tester", "base", "adaptive", "csv"):
        p.add_argument(name, type=Path)
    args = p.parse_args()
    gen, tester, base, adaptive = (x.resolve() for x in
                                   (args.gen, args.tester, args.base, args.adaptive))
    with tempfile.TemporaryDirectory() as tmp, args.csv.open("w", newline="") as f:
        work = Path(tmp)
        script = Path(__file__).resolve()
        seedfile = work / "seeds.txt"
        trace = work / "trace.txt"
        writer = csv.writer(f, lineterminator="\n")
        writer.writerow(("split", "seed", "mode", "score", "h0_area", "h0_pets",
                         "walls", "direct_seconds", "relay_seconds"))
        for split in ("dev", "holdout"):
            seedfile.write_text("".join(f"{s}\n" for s in seeds(split)))
            subprocess.run([gen, seedfile], cwd=work, check=True)
            for i, seed in enumerate(seeds(split)):
                case = (work / "in" / f"{i:04}.txt").read_bytes()
                modes = [("base", base), ("adaptive", adaptive)]
                for mode, binary in (modes if i % 2 == 0 else modes[::-1]):
                    before = time.monotonic()
                    plain = subprocess.run([tester, binary], input=case, capture_output=True,
                                           check=True, timeout=3)
                    direct_sec = time.monotonic() - before
                    before = time.monotonic()
                    proxy = subprocess.run([tester, sys.executable, script, "--relay", binary, trace],
                                           input=case, capture_output=True,
                                           check=True, timeout=3)
                    relay_sec = time.monotonic() - before
                    assert direct_sec < 3 and relay_sec < 3
                    assert plain.stdout == proxy.stdout
                    score, *extra = replay(case.decode(), trace.read_text().splitlines(),
                                          plain.stdout.decode().splitlines())
                    assert plain.stderr == proxy.stderr
                    match = re.fullmatch(rb"Score = (\d+)\s*", plain.stderr)
                    assert match and int(match[1]) == score and score > 0
                    writer.writerow((split, seed, mode, score, *extra,
                                     f"{direct_sec:.6f}", f"{relay_sec:.6f}"))
                    f.flush()


if __name__ == "__main__":
    if len(sys.argv) == 4 and sys.argv[1] == "--relay":
        relay(Path(sys.argv[2]), Path(sys.argv[3]))
    else:
        main()
