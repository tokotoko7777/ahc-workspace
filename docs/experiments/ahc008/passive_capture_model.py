"""First legal closing opportunity for one passive random walker in static AHC008 traps.

Walls and fire-ready outside guards are assumed present at time zero. No other
pets, competing doors, construction delay, or subsequent captures are modeled.
Probabilities propagate the official 1/2/3 unbiased nearest-neighbor substeps,
checking for a safe closure only at turn boundaries. Requires NumPy.
"""
import argparse
from dataclasses import dataclass

import numpy as np

SIZE = 30
STEPS = [(1, 0), (-1, 0), (0, 1), (0, -1)]


def near(p):
    x, y = p
    for dx, dy in STEPS:
        if 0 <= x + dx < SIZE and 0 <= y + dy < SIZE:
            yield x + dx, y + dy


@dataclass
class Geometry:
    walls: set
    doors: set
    inside: set

    def safe(self):
        for d in self.doors:
            assert any(q not in self.inside | self.walls | self.doors for q in near(d))
        for p in self.inside:
            assert set(near(p)) <= self.walls | self.doors | self.inside
        forbidden = set().union(*(set(near(p)) for p in self.doors))
        return self.inside - forbidden


def corridor(length, n):
    """Transits through middle separator; bypass retained along far edge."""
    a, b = 15 - length // 2, 15 + (length - 1) // 2
    rows = [((i + 1) * 26 // (n + 1)) for i in range(n)]
    wall = {(r, 15) for r in range(28) if r not in rows}
    doors, inside = set(), set()
    for r in rows:
        wall.update((rr, c) for rr in (r - 1, r + 1) for c in range(a, b + 1))
        doors.update({(r, a), (r, b)})
        inside.update((r, c) for c in range(a + 1, b))
    return Geometry(wall, doors, inside)


def edge_pen(depth, width, n):
    """Equal-size pens with distinct entrances along one edge."""
    wall, doors, inside = set(), set(), set()
    for i in range(n):
        r = i * (width + 1)
        if r + width >= SIZE:
            raise ValueError("too many pens on one edge")
        wall.update((rr, depth) for rr in range(r, r + width))
        wall.update((r + width, c) for c in range(depth + 1))
        if i:
            wall.update((r - 1, c) for c in range(depth + 1))
        d = (r + width // 2, depth)
        wall.remove(d)
        doors.add(d)
        inside.update((rr, c) for rr in range(r, r + width) for c in range(depth))
    return Geometry(wall, doors, inside)


def absorption(geom, turns):
    safe = geom.safe()
    coords = [(r, c) for r in range(SIZE) for c in range(SIZE) if (r, c) not in geom.walls]
    node = {p: i for i, p in enumerate(coords)}
    orig, dest, prob = [], [], []
    degrees = np.array([sum(q in node for q in near(p)) for p in coords], dtype=float)
    for i, p in enumerate(coords):
        for q in near(p):
            if q in node:
                orig.append(i)
                dest.append(node[q])
                prob.append(1.0 / degrees[i])
    orig, dest, prob = map(np.array, (orig, dest, prob))
    eligible = [node[p] for p in safe]
    out = {}
    for pace in (1, 2, 3):
        d = degrees / degrees.sum()  # exact stationary distribution, not uniform
        result = {}
        for turn in range(max(turns) + 1):
            d[eligible] = 0
            if turn in turns:
                result[turn] = 1 - d.sum()
            if turn < max(turns):
                for _ in range(pace):
                    d = np.bincount(dest, weights=d[orig] * prob, minlength=len(coords))
        out[pace] = result
    return out


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--turns", nargs="+", type=int, default=[50, 100, 200, 300])
    args = parser.parse_args()
    design = {f"{n} tunnel L{length}": corridor(length, n) for n in (1, 2, 4, 6) for length in (5, 7, 9)}
    design.update({f"{n} edge {d}x{w}": edge_pen(d, w, n) for n in (1, 2, 4) for d, w in ((3, 3), (4, 4), (6, 6))})
    print("geometry,walls,doors,inside,safe,pace," + ",".join(f"turn_{x}" for x in args.turns))
    for label, g in design.items():
        for p, it in absorption(g, args.turns).items():
            print(f"{label},{len(g.walls)},{len(g.doors)},{len(g.inside)},{len(g.safe())},{p}," +
                  ",".join(f"{it[t]:.6f}" for t in args.turns))
