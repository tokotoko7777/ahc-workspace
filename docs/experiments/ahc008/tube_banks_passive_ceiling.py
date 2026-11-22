"""Prebuilt single-passive-animal arrival ceiling across repeated tube banks.

No construction waits, all gates anytime staffed, no other animals; assume
all wall rows are completed at t=0 and capture at first observed visit to
either safe inner cell. Not a whole-game capture forecast.
"""
from collections import defaultdict
from itertools import product
import numpy as np
from tube_banks_geometry import adj, BOARD, D, Geometry


def ceiling(bank, turns=(120, 200)):
    wall, tubes=bank.make()
    sat=set().union(*(q[3] for q in tubes))
    sites=sorted(BOARD-wall)
    idx={q:k for k,q in enumerate(sites)}
    n=len(idx)
    nb=[tuple(idx[q] for q in adj(p) if q in idx) for p in sites]
    ori=np.array([i for i, li in enumerate(nb) for _ in li], dtype=int)
    dest=np.array([j for li in nb for j in li], dtype=int)
    p=np.array([1./len(li) for li in nb for _ in li])
    safe=[idx[q] for q in sat]
    ret=[]
    for pace in (1,2,3):
        s=np.full(n,1./n)
        for t in range(max(turns)+1):
            s[safe]=0.
            if t in turns:
                ret.append(1.-s.sum())
            if t<max(turns):
                for _ in range(pace):
                    s=np.bincount(dest,weights=s[ori]*p,minlength=n)
    return ret


def run():
    print('B,k,pattern,collar,W,clean_after_15_one_each,cow120,cow200,pig120,pig200,rabbit120,rabbit200')
    for B, k in ((b,k) for b,ks in ((2,range(8,14)),(3,range(7,14)),(4,range(6,11))) for k in ks):
        lanes=[(D-1-7*B)//2+7*x for x in range(B+1)]
        p=(D-1-2*k)//2
        def split(q):
            top=k//2+q
            return [(1,top),(D-2-2*(k-top),k-top)]
        g={
            'aligned': [[(p,k)] for _ in range(B)],
            'stagger': [[(p+(i%2),k)] for i in range(B)],
        }
        if k<=12:
            g['split']=[split(i%2) for i in range(B)]
        for (mode,runs),collar in product(g.items(),(True,False)):
            layout=Geometry(lanes,runs,collar)
            wall,tubes=layout.make()
            for a,b in zip(runs[:-1],runs[1:]):
                assert a is not b
            res=ceiling(layout)
            print(B,k,mode,int(collar),len(wall),900-len(wall)-90,*(f'{x:.6f}' for x in res),sep=',')
if __name__=='__main__':run()
