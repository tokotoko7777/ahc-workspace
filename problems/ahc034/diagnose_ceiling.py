"""Measure independent lower bounds on 20 fresh AHC034 development inputs."""

import argparse
import csv
import hashlib
import itertools
import json
import re
import statistics
import subprocess
import tempfile
from pathlib import Path

MOVE = {b"U": (-1, 0), b"D": (1, 0), b"L": (0, -1), b"R": (0, 1)}


def manhattan_mst(points):
    """Dense Prim on the required points (including the origin)."""
    n = len(points)
    used = [False] * n
    distance = [10**9] * n
    distance[0] = 0
    total = 0
    for _ in points:
        i = min((j for j in range(n) if not used[j]), key=distance.__getitem__)
        used[i] = True
        total += distance[i]
        r, c = points[i]
        for j, (a, b) in enumerate(points):
            if not used[j]:
                distance[j] = min(distance[j], abs(r - a) + abs(c - b))
    return total


def bounds(n, height):
    points = [divmod(i, n) for i, h in enumerate(height) if i == 0 or h]
    even = sum((r + c) % 2 == 0 for r, c in points)
    odd = len(points) - even
    moves = max(2 * (even - 1), 2 * odd - 1, manhattan_mst(points))
    row_sums = (sum(height[r * n : (r + 1) * n]) for r in range(n))
    col_sums = (sum(height[c::n]) for c in range(n))
    transport = sum(
        abs(v) for sums in (row_sums, col_sums) for v in itertools.accumulate(sums)
    )
    return moves, transport


def replay(n, initial, output):
    """Check a completed solution, and split cost into its three terms."""
    height = initial.copy()
    moves = transport = handling = load = r = c = 0
    commands = output.splitlines()
    assert len(commands) <= 100_000
    for command in commands:
        if command in MOVE:
            dr, dc = MOVE[command]
            r, c = r + dr, c + dc
            assert 0 <= r < n and 0 <= c < n
            moves += 1
            transport += load
        else:
            assert re.fullmatch(rb"[+-][1-9][0-9]*", command)
            change = int(command)
            assert abs(change) <= 1_000_000
            handling += abs(change)
            load += change
            height[r * n + c] -= change
            assert load >= 0
    assert not any(height) and load == 0
    return moves, transport, handling


def score(base, cost):
    return (2 * 10**9 * base + cost) // (2 * cost)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("gen", "vis", "solver", "work_dir"):
        parser.add_argument(name, type=lambda s: Path(s).resolve())
    args = parser.parse_args()
    args.work_dir.mkdir(parents=True, exist_ok=True)
    seeds = [
        int.from_bytes(
            hashlib.sha256(f"ahc034/lower-bound-diagnostic-v1/{i}".encode()).digest()[:8],
            "little",
        )
        for i in range(20)
    ]
    rows = []
    with tempfile.TemporaryDirectory(dir=args.work_dir) as directory:
        work = Path(directory)
        seed_file = work / "seeds.txt"
        seed_file.write_text("".join(f"{s}\n" for s in seeds), encoding="ascii")
        subprocess.run([args.gen, seed_file], cwd=work, check=True, timeout=60)
        for i, seed in enumerate(seeds):
            case = work / "in" / f"{i:04}.txt"
            n, *height = map(int, case.read_text(encoding="ascii").split())
            base = sum(map(abs, height))
            assert n > 0 and len(height) == n * n and base > 0 and not sum(height)
            min_moves, min_transport = bounds(n, height)
            output = subprocess.check_output(
                [args.solver], input=case.read_bytes(), cwd=work, timeout=2
            )
            answer = work / "answer.txt"
            answer.write_bytes(output)
            scored = subprocess.run(
                [args.vis, case, answer], cwd=work, capture_output=True,
                text=True, check=True, timeout=10
            )
            moves, transport, handling = replay(n, height, output)
            actual = score(base, handling + 100 * moves + transport)
            ceiling = score(base, base + 100 * min_moves + min_transport)
            assert moves >= min_moves and transport >= min_transport and handling >= base
            assert actual <= ceiling
            matching = re.search(r"Score\s*=\s*(\d+)", scored.stdout + scored.stderr)
            assert matching and int(matching.group(1)) == actual
            rows.append(dict(
                seed=seed, score=actual, ceiling=ceiling, B=base, L=min_moves,
                W=min_transport, moves=moves, transport=transport, handling=handling,
                road_gap=100 * (moves - min_moves), cargo_gap=transport - min_transport,
            ))
    with (args.work_dir / "comparison.csv").open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    print(json.dumps({
        key: statistics.mean(row[key] for row in rows)
        for key in rows[0] if key != "seed"
    }, indent=2))


if __name__ == "__main__":
    main()
