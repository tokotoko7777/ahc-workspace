"""Conditional AHC008 model: two gates 0,5; four interior cells 1..4.
Open gates have degree four. Leaving through an exterior neighbor ends
the attempt. One guard is ready at t=0; the second arrives after ETA.
No other pets, nonlocal returns, staffing or setup effects are modeled.
"""
from functools import cache
from collections import defaultdict


@cache
def move(pace, early, x):
    state={x:1.}
    for _ in range(pace):
        nxt=defaultdict(float)
        for p,q in state.items():
            if p==0:
                nxt[1]+=q/4
            elif p==5:
                nxt[4]+=q/4
            elif p==1 and early:
                nxt[2]+=q
            else:
                nxt[p-1]+=q/2
                nxt[p+1]+=q/2
        state=nxt
    return tuple(state.items())


def close_probability(pace, start, eta, wait, early):
    d={start:1.}
    safe=(1,2,3) if early else (2,3)
    cap=0.
    for h in range(eta+wait+1):
        if h>=eta:
            cap+=sum(d.pop(s,0) for s in safe)
        nd=defaultdict(float)
        for x,p in d.items():
            for y,q in move(pace,early,x):
                nd[y]+=p*q
        d=nd
    return cap


if __name__=='__main__':
    print('pace,start,eta,extra_wait,together,stage_left,delta')
    for k in (1,2,3):
        for x in (2,3,4):
            for eta in (1,2,3,4):
                for w in (0,2):
                    a=close_probability(k,x,eta,w,False)
                    b=close_probability(k,x,eta,w,True)
                    print(k,x,eta,w,f'{a:.5f}',f'{b:.5f}',f'{b-a:.5f}',sep=',')
