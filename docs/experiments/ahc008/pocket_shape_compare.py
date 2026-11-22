"""AHC008 same-area one-gate comparison on a 30x30 open board.

Assumes walls built and an exterior guard standing at the gate. Only one
passive walker; movement follows official uniform legal 1/2/3 substeps per
turn. The guard can close even after a previous exit and re-entry.
"""
from collections import defaultdict

D = 30
GATE = (0, 10)
SHAPES = {
    "line": {(0, 11), (0, 12), (0, 13), (0, 14)},
    "square": {(0, 11), (0, 12), (1, 11), (1, 12)},
    "L": {(0, 11), (0, 12), (0, 13), (1, 11)},
}


def near(p):
    for dx, dy in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
        q = p[0] + dx, p[1] + dy
        if 0 <= q[0] < D and 0 <= q[1] < D:
            yield q


def geometry(inside, gate):
    wall = {q for p in inside for q in near(p) if q not in inside and q != gate}
    safe = inside - set(near(gate))
    graph = {(i, j): tuple(x for x in near((i, j)) if x not in wall)
             for i in range(D) for j in range(D) if (i, j) not in wall}
    assert {p for p in graph[gate] if p in inside} == {(0, 11)}
    return graph, wall, safe


def walk(graph, start, speed, targets, horizons, check_between=False):
    cur = {start: 1.0}
    total = 0.
    for t in range(1, max(horizons) + 1):
        for _ in range(speed):
            nxt = defaultdict(float)
            for p, v in cur.items():
                x = v / len(graph[p])
                for q in graph[p]:
                    if check_between and q in targets:
                        total += x
                    else:
                        nxt[q] += x
            cur = nxt
        if not check_between:
            total += sum(cur.pop(s, 0.) for s in targets)
        if t in horizons:
            assert abs(total + sum(cur.values()) - 1) < 1e-10
            yield total


def compare():
    horizons = [8, 20]
    print("shape,prep_walls,start,speed,turn,ever_entered,ever_legally_closed")
    for label, inside in SHAPES.items():
        graph, wall, safe = geometry(inside, GATE)
        assert len(wall) == 5  # Identical geometry effort: each touches board edge.
        for initial in [(0, 9), (1, 9), (0, 10)]:
            for pace in range(1, 4):
                entered = walk(graph, initial, pace, inside, horizons, True)
                closed = walk(graph, initial, pace, safe, horizons)
                for t,e,c in zip(horizons, entered, closed, strict=True):
                    assert c <= e + 1e-12
                    print(f"{label},{len(wall)},({initial[0]};{initial[1]}),{pace},{t},{e:.6f},{c:.6f}")


if __name__ == "__main__":
    compare()
