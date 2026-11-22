"""Exact single-dog chances for a sealed 1-cell-wide, two-door transit trap.

Assumes prebuilt side walls and two humans in exterior building stance. The
dog is known to target the far-side human, who remains fixed until capture or
until the dog first touches that human. Other pets, wall placement delays and
the interaction of real shortest-path alternatives are not simulated here.

Positions 1..length are the open hallway; its endpoints are the two pending
walls. A=0 and B=length+1 are exterior building stances. An alternate A--B
route models the maintained connection outside the trap.
"""
import argparse
from collections import deque
from fractions import Fraction


def forecast(length, bypass, start, turns):
    a, b = 0, length + 1
    nodes = [[i - 1, i + 1] for i in range(b + bypass)]
    nodes[a][0] = len(nodes) - 1
    nodes[-1][1] = a
    door_safe = set(range(3, length - 1))
    dist = {b: 0}
    q = deque([b])
    while q:
        for nxt in nodes[q.popleft()]:
            if nxt not in dist:
                dist[nxt] = min(dist[v] for v in nodes[nxt] if v in dist) + 1
                q.append(nxt)

    p = {start: Fraction(1)}
    captured = reset = Fraction(0)
    def add(bucket, k, value):
        bucket[k] = bucket.get(k, Fraction(0)) + value

    for t in range(turns + 1):
        for k in door_safe:
            captured += p.pop(k, Fraction(0))
        reset += p.pop(b, Fraction(0))
        if t == turns:
            break
        after = {}
        for k, prob in p.items():
            shortest = [v for v in nodes[k] if dist[v] < dist[k]]
            for mid in shortest:
                m = prob / len(shortest)
                # Official behavior resets the target as soon as the
                # directed substep lands on it, even when the random step
                # subsequently leaves; all such paths leave this model.
                if mid == b:
                    reset += m
                    continue
                for end in nodes[mid]:
                    add(after, end, m / len(nodes[mid]))
        p = after
    assert captured + reset + sum(p.values()) == 1
    return captured, reset


def test():
    for turns in range(1, 7):
        assert forecast(6, 9, 0, turns)[0] == 1 - (turns + 1) * Fraction(1, 2) ** turns
        assert forecast(6, 9, 1, turns)[0] == 1 - Fraction(1, 2)**turns
        assert forecast(5, 9, 0, turns)[0] == 0
        assert forecast(5, 9, 1, turns)[0] == 1 - Fraction(1, 2)**turns
        assert forecast(6, 6, 0, turns)[0] == 0


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--self-test', action='store_true')
    args = ap.parse_args()
    if args.self_test:
        test()
        print('exact two-step parity, both entrance phases and shorter-bypass cases checked')
    else:
        print('door_span,interior_area,bypass,start,turns,captured,reset')
        for length in [4, 5, 6, 7, 8]:
            for bypass in [length, length+1, length+2, length+5]:
                for start in [0, 1]:
                    for turns in [1, 2, 4, 8]:
                        cap, reset = forecast(length, bypass, start, turns)
                        print(length, length-2, bypass, start, turns, float(cap), float(reset), sep=',')
