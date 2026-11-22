"""Separate 3/4-cell banks with globally complementary closure parity."""
import numpy as np
from tube_banks_geometry import BOARD,D,adj,flood

class Mixed:
    def __init__(self,widths,shift,rows=tuple(range(3,26,2)),stagger=False):
        self.widths=widths
        self.shift=shift
        self.rows=rows
        self.stagger=stagger
    def make(self):
        a=self.shift
        walls=set()
        tubes=[]
        for j,w in enumerate(self.widths):
            assert w in (3,4)
            for r0 in self.rows:
                r=r0+((j+1)%2 if self.stagger else 0)
                body={(r,a+k) for k in range(2,w+2)}
                gates={(r,a+1),(r,a+w+2)}
                stances={(r,a),(r,a+w+3)}
                walls.update((r+y,c) for _,c in body for y in [-1,1])
                safe=body-{q for g in gates for q in adj(g)}
                assert len(safe)==w-2
                tubes.append((body,gates,stances,safe))
            a+=w+3
        inside=set().union(*(t[0] for t in tubes))
        doors=set().union(*(t[1] for t in tubes))
        stances=set().union(*(t[2] for t in tubes))
        assert (inside|doors|stances|walls)<=BOARD
        assert not walls&(inside|doors|stances)
        assert len(inside)==sum(len(t[0]) for t in tubes)
        assert len(doors)==2*len(tubes)
        assert flood((0,0),walls|doors)==BOARD-walls-doors-inside
        for b,g,h,s in tubes: assert flood(next(iter(b)),walls|doors)==b
        return walls,tubes

def passive(walls,tubes,pace,parity,ends=(120,200)):
    sites=sorted(BOARD-walls)
    idx={p:i for i,p in enumerate(sites)}
    edges=[(idx[p],idx[q],1.0/len(ns)) for p in sites
           for ns in [list(q for q in adj(p) if q in idx)] for q in ns]
    src,dst,rates=zip(*edges)
    src=np.array(src);dst=np.array(dst);rates=np.array(rates)
    safe=[idx[p] for _,_,_,bs in tubes for p in bs]
    prob=np.array([1. if parity is None or sum(p)%2==parity else 0. for p in sites])
    prob/=prob.sum()
    ans=[]
    for t in range(max(ends)+1):
        prob[safe]=0
        if t in ends:ans.append(1-prob.sum())
        if t<max(ends):
            for _ in range(pace):prob=np.bincount(dst,weights=rates*prob[src],minlength=len(sites))
    return ans

if __name__=='__main__':
    print('config,shift,stagger,braces,tubes,compat0,compat1,cow120,cow200,pig0_120,pig0_200,pig1_120,pig1_200,rabbit120,rabbit200')
    for ws in ((4,4,4),(3,4,4),(3,4,3),(3,3,3)):
        a=(D-1-sum(w+3 for w in ws))//2
        for st in (False,True):
            walls,tubes=Mixed(ws,a,stagger=st).make()
            typ=[(1,None),(2,0),(2,1),(3,None)]
            counts=[sum(any(sum(p)%2==q for p in s) for _,_,_,s in tubes) for q in (0,1)]
            row=[ ''.join(map(str,ws)), a,int(st),len(walls),len(tubes),*counts]
            row += [f'{p:.6f}' for k,c in typ for p in passive(walls,tubes,k,c)]
            print(*row,sep=',')
