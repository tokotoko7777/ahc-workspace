"""Conditional AHC008 closure opportunity for animals in prepared 3×3 sleeve.

Both latched gate stances are occupied by builders outside the center. There
are no other pets near the latches; treat entry into either outer wing as
escape, not entry into a latch square. Pig and cow walks follow fixed
official substep counts. No arrivals, construction or routing are modeled.
"""
from collections import defaultdict
from itertools import product
from math import prod

inside = set(product(range(3), range(3)))
gates = {(1, -1), (1, 3)}
wing = {(1, -2), (1, 4)}
open_ = inside | gates
safe = inside - {(1, 0), (1, 2)}
adjacent = lambda a, b: sum(abs(x - y) for x, y in zip(a, b)) == 1
adj = {x: [y for y in open_ | wing if adjacent(x, y)] for x in open_}


def turn_transition(phase, x):
    start = {x: 1.0}
    for _ in range(phase):
        dest = defaultdict(float)
        for a, p in start.items():
            for b in adj[a]:
                if b not in wing:
                    dest[b] += p / len(adj[a])
        start = dest
    return list(start.items())


def chances(init, speed, n):
    assert len(speed) == len(init)
    kernel = {k: {x: turn_transition(k, x) for x in open_} for k in set(speed)}
    current = {tuple(init): 1.0}
    result = []
    captured = 0.0
    for t in range(n + 1):
        distinct = [pos for pos in current if all(p in safe for p in pos)]
        for pos in distinct:
            captured += current.pop(pos)
        total = sum(current.values())
        result.append((t, captured, 1 - captured - total))
        if t == n:
            break
        new = defaultdict(float)
        for pos, p in current.items():
            for path in product(*(kernel[k][x] for x, k in zip(pos, speed))):
                new[tuple(x for x, _ in path)] += p * prod(f for _, f in path)
        current = new
    return result


if __name__ == "__main__":
    w, e, center, g = (1, 0), (1, 2), (0, 1), (1, -1)
    assert abs(chances([w], [1], 45)[-1][1] - 6/7) < 1e-9
    assert abs(chances([w], [2], 45)[-1][1] - 3/4) < 1e-9
    assert abs(chances([g], [2], 45)[-1][1] - 3/7) < 1e-9
    assert chances([center], [2], 0)[0] == (0, 1, 0)
    print('starts,speeds,turns,capture,escape')
    for pos in ([w], [g], [w,e], [w,center], [w,e,center]):
        for speed in [(1,)*len(pos), (2,)*len(pos), (3,)*len(pos)]:
            for n, a, b in chances(pos,speed,15):
                if n in (1, 2, 4, 8, 15):
                    print(f'"{pos}","{speed}",{n},{a:.6f},{b:.6f}')
