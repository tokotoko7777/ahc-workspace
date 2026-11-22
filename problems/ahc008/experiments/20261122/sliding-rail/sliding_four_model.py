"""A conditional two-guard exact-four 1D transit model for AHC008.

Open one-cell board-edge rail is positions 0..n-1 with fully built lateral
walls. Both ends remain open into the main board. Guards move along the
narrow rail from exterior stances -1 and n, and do not block animal movement.
Animal escape into the wider board is absorbing. 1/2/3 uniform substeps
represent cow/pig/rabbit. Only the observed full-turn state is available.
The model omits rail construction time, animal arrival, other agents and paths
outside the rail, so it is not a whole-game capture-rate claim.
"""
from functools import cache
from collections import defaultdict
from statistics import mean


def study(n, pace, turns, starts=None):
    stance_pairs = [(g-1, g+6) for g in range(n-5)]
    assert all(b-a == 7 for a,b in stance_pairs)
    starts = list(range(n)) if starts is None else starts
    def transition(p):
        d = {p: 1.0}
        for _ in range(pace):
            e = defaultdict(float)
            for x, f in d.items():
                for q in (x-1,x+1):
                    if 0<=q<n:
                        e[q] += .5*f
            d = e
        return tuple(d.items())
    step = [transition(p) for p in range(n)]
    def can_finish(a, b, p):
        gates = a+1, b-1
        return (a,b) in stance_pairs and gates[0]+2 <= p <= gates[1]-2

    @cache
    def adaptive(a, b, p, left):
        if can_finish(a,b,p):
            return 1.
        if not left:
            return 0.
        return max(sum(prob*adaptive(a1,b1,q,left-1) for q,prob in step[p])
                   for a1 in range(max(-1,a-1), min(n,a+1)+1)
                   for b1 in range(max(-1,b-1,a1+1), min(n,b+1)+1))

    @cache
    def fixed(a, b, p, left, goal):
        if (a,b)==goal and can_finish(a,b,p):
            return 1.
        if not left:
            return 0.
        a1 = a + (goal[0]>a)-(goal[0]<a)
        b1 = b + (goal[1]>b)-(goal[1]<b)
        return sum(prob*fixed(a1,b1,q,left-1,goal) for q,prob in step[p])

    dynamic = [adaptive(-1,n,p,turns) for p in starts]
    static = [max(fixed(-1,n,p,turns,g) for g in stance_pairs) for p in starts]
    assert all(x+1e-12>=y for x,y in zip(dynamic,static))
    return dynamic, static


if __name__ == '__main__':
    d,s = study(6,1,0)
    assert d == s == [0,0,1,1,0,0], (d,s)
    print('rail,species,turns,best_fixed_window,dynamic_window,delta')
    for n in (6,8,10,12):
        for k in (1,2,3):
            for t in (3,6,10):
                d,s = study(n,k,t)
                if n==6: assert abs(mean(s)-mean(d))<1e-12
                print(f'{n},{k},{t},{mean(s):.6f},{mean(d):.6f},{mean(d)-mean(s):.6f}')
