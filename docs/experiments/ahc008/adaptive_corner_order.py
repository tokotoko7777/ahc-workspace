"""AHC008 2x2 corner: optimize wall order vs straightforward greedy.

Four distinct builders already occupy exterior stances of the four pending
wall cells; each can build its own wall. One passive animal of known pace,
with only its observed whole-turn positions available. Only the first 8×8
part of the corner is tracked, and moving out of that region ends the attempt.
Walls physically affect every subsequent substep. This is a strong staffing
fixture, not a live solver or a distribution claim.
"""
from collections import defaultdict
from functools import cache
from statistics import mean

D = 8
PEN = {(0, 0), (0, 1), (1, 0), (1, 1)}
WALL = [(0, 2), (1, 2), (2, 0), (2, 1)]
BUILDER = [(0, 3), (1, 3), (3, 0), (3, 1)]
FULL = (1 << len(WALL)) - 1


def md(a, b):
    return sum(abs(x-y) for x, y in zip(a, b))


assert all(md(g, w) == 1 for g, w in zip(BUILDER, WALL))
assert not set(BUILDER) & (PEN | set(WALL))
assert set(q for p in PEN for q in WALL if md(p, q) == 1) == set(WALL)


@cache
def options(mask, pet):
    legal = sum(1 << i for i, pos in enumerate(WALL) if not (mask >> i & 1) and md(pos, pet) >= 2)
    ans = []
    add = legal
    while True:
        upgraded = add | mask
        if upgraded != FULL or pet in PEN:
            ans.append(upgraded)
        if add == 0:
            break
        add = (add - 1) & legal
    return tuple(ans)


@cache
def transition(mask, start, speed):
    cur = {start: 1.}
    walls = {w for i,w in enumerate(WALL) if mask >> i & 1}
    for _ in range(speed):
        nxt = defaultdict(float)
        for (r,c), prob in cur.items():
            nbs = [(u,v) for u,v in ((r-1,c),(r+1,c),(r,c-1),(r,c+1))
                   if min(u,v) >= 0 and (u,v) not in walls]
            for p in nbs:
                if max(p) < D:
                    nxt[p] += prob/len(nbs)
        cur = nxt
    return tuple(cur.items())


def solve(pace, horizon, init_mask):
    @cache
    def dp(mask, pet, left, mode):
        if mask == FULL:
            return float(pet in PEN)
        if not left:
            return 0.
        opt = options(mask,pet)
        if mode == 1:
            opt = (max(opt, key=lambda nxt: (nxt.bit_count(), -min(
                (md(pet, p) for i,p in enumerate(WALL) if not (nxt>>i&1)),default=0), -nxt)),)
        return max(1. if w==FULL else sum(prob * dp(w,p,left-1,mode)
                                         for p,prob in transition(w,pet,pace))
                   for w in opt)

    mask = init_mask
    near = [(r,c) for r in range(D) for c in range(D)
            if 2 <= r+c <= 5 and (r,c) not in PEN and
            (r,c) not in [p for i,p in enumerate(WALL) if mask>>i&1]]
    # Tested from outside, where scheduling matters. From the far corner
    # itself, schedule or species doesn't change an immediately legal close.
    fixed = [dp(mask,p,horizon,1) for p in near]
    optimal = [dp(mask,p,horizon,0) for p in near]
    assert all(q+1e-12>=p for p,q in zip(fixed,optimal))
    assert dp(0,(0,0),1,0) == dp(0,(0,0),1,1) == 1.
    return mean(fixed),mean(optimal),max(o-f for f,o in zip(fixed,optimal))


if __name__ == '__main__':
    print('starting_walls,pace,turns,greedy,dynamic,largest_startwise_delta')
    for mask in (0,1,3,5):
        for speed in (1,2,3):
            for limit in (3,6,9):
                a,b,c=solve(speed,limit,mask)
                print(f'{mask:04b},{speed},{limit},{a:.6f},{b:.6f},{c:.6f}')
