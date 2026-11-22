"""Enumerate edge-supported one-gate AHC008 passive pet pens of <=8 cells.

This is a conditional motif catalog. All shell walls must be already present,
an exterior builder stationary, and only one animal of known pace is modeled.
Any transition to an outside square beyond the gate ends the attempt, while a
pet stepping on the gate and back can still be caught. Gate legality is tested
only at turn boundaries. No candidate exploits seed or future observations.
"""
import argparse
from typing import TypeAlias

Point: TypeAlias = tuple[int, int]
DIR = ((-1, 0), (1, 0), (0, -1), (0, 1))


def neighbors(p):
    return [(p[0]+dx, p[1]+dy) for dx, dy in DIR if p[0]+dx >= 0]


def shape_id(s):
    a = min(x for x, _ in s)
    b = min(y for _, y in s)
    c = max(y for _, y in s)
    return min(tuple(sorted((x-a, y-b) for x, y in s)),
               tuple(sorted((x-a, c-y) for x, y in s)))


def all_outside_connected(shape, shell, gate, outside):
    # The game board edge is x=0; escaping a bounding rectangle around the
    # closed shape implies a path to main without using the gate.
    x_max = max(x for x, _ in shape) + 2
    y_min = min(y for _, y in shape) - 2
    y_max = max(y for _, y in shape) + 2
    excluded = set(shape) | shell | {gate}
    def escapes(start):
        todo = [start]; seen = {start}
        for at in todo:
            x, y = at
            if x >= x_max or y <= y_min or y >= y_max:
                return True
            for q in neighbors(at):
                if q not in excluded and q not in seen:
                    seen.add(q)
                    todo.append(q)
        return False
    return all(map(escapes, outside))


def motif_prob(shape, gate, mouth, out, turns):
    nodes = list(shape) + [gate]
    idx = {p: i for i, p in enumerate(nodes)}
    near = [list(idx[q] for q in neighbors(p) if q in idx) for p in nodes]
    inv_deg = [1 / (len(adj) + (out if p == gate else 0)) for p, adj in zip(nodes, near)]
    safe = [idx[p] for p in shape - set(neighbors(gate))]
    def forecast(steps):
        dp = [0.] * len(nodes); dp[idx[mouth]] = 1.
        captured = 0.
        for _ in range(turns):
            for _ in range(steps):
                new = [0.] * len(nodes)
                for i, v in enumerate(dp):
                    if v:
                        p = v * inv_deg[i]
                        for j in near[i]:
                            new[j] += p
                dp = new
            for i in safe:
                captured += dp[i]
                dp[i] = 0.
        return captured
    return tuple(map(forecast, (1, 2, 3)))


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--max-area', type=int, default=8)
    ap.add_argument('--turns', type=int, default=8)
    p = ap.parse_args()
    assert tuple(round(x, 4) for x in
                 motif_prob({(0,0),(0,1),(1,0),(1,1)}, (0,2), (0,1), 1, 8)
                 ) == (0.7994, 0.6641, 0.7246)
    shapes = {((0, 0),)}
    pareto = {}
    for area in range(1, p.max_area+1):
        count = 0
        for key in shapes:
            shape = set(key)
            shell = set(q for s in shape for q in neighbors(s)) - shape
            for g in shell:
                mouth = [v for v in neighbors(g) if v in shape]
                if len(mouth) != 1:
                    continue
                outside = [q for q in neighbors(g) if q not in shape|shell]
                out = len(outside)
                if not out or not all_outside_connected(shape, shell, g, outside):
                    continue
                probs = motif_prob(shape, g, mouth[0], out, p.turns)
                count += 1
                if area < 2:   # An occupied pet cannot be left without a legal next move.
                    continue
                h = (area, len(shell)-1)
                compare = (min(probs), probs)
                if h not in pareto or compare > pareto[h][0]:
                    pareto[h] = compare, (tuple(sorted(shape)), g, mouth[0], out)
        print('area',area,'shapes',len(shapes),'touched_candidates',count)
        if area < p.max_area:
            # Grow in every direction before re-anchoring; the border of a
            # subset of a motif need not be the border of the full motif.
            shapes = {shape_id(set(key)|{(x+dx,y+dy)}) for key in shapes for x,y in key
                      for dx,dy in DIR if (x+dx,y+dy) not in key}
    for (a, w), (val, pose) in sorted(pareto.items()):
        # Domination only across <= area AND <= prep walls.
        if any((b <= a and v <= w and (b, v) != (a, w) and cand[0] >= val[0]-1.e-12)
               for (b,v), (cand,_) in pareto.items()):
            continue
        print('pareto',a,w,*[f'{v:.4f}' for v in val[1]],pose)


if __name__ == '__main__':
    main()
