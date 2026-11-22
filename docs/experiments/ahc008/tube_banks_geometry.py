"""Check repeated A(=)B(=)C(=)D exact-four layout + score area budget.

Interpretation of user's structural hint, not another participant's source.
Persistent aisle columns distance 7; tube interiors and gates are horizontal.
Both ends use an open perpendicular crossbar. Walls are counted per square,
shared between adjacent tubes in the same bank. No gameplay forecast here.
"""
from itertools import product
from dataclasses import dataclass
from collections import deque

D = 30
BOARD = set(product(range(D), repeat=2))


def adj(q):
    r, c = q
    for p in ((r-1,c),(r+1,c),(r,c-1),(r,c+1)):
        if p in BOARD:
            yield p


def flood(start, wall):
    seen = {start}; todo = deque(seen)
    while todo:
        for q in adj(todo.popleft()):
            if q not in seen and q not in wall:
                seen.add(q); todo.append(q)
    return seen


@dataclass
class Geometry:
    lanes: list
    banks: list
    collar: bool=True

    def make(self):
        wall, tubes = set(), []
        for a, b, runs in zip(self.lanes[:-1], self.lanes[1:], self.banks, strict=True):
            assert b-a == 7
            for start,k in runs:
                assert 0 < start <= start+2*k < D-1 and k>0
                for i in range(k+1):
                    extra = 1 if self.collar else 2
                    wall |= {(start+2*i,c) for c in range(a+extra,b-extra+1)}
                for v in range(k):
                    r=start+2*v+1
                    pen={(r,c) for c in range(a+2,b-1)}
                    gates={(r,a+1),(r,b-1)}
                    guards={(r,a),(r,b)}
                    safe=pen-{q for p in gates for q in adj(p)}
                    assert len(pen)==4 and len(safe)==2
                    tubes.append((pen,gates,guards,safe))
        union=set().union(*(t[0]|t[1] for t in tubes))
        assert len(union)==6*len(tubes) and not wall&union
        return wall,tubes


def run():
    print('collar,banks,tubes_per_bank,setup,actual_room_capacity,largest_aisle_component_if_all_shut,residual_area_if_10_cages,residual_area_if_15_cages,residual_area_if_20_cages')
    for B,ks in ((2,range(10,14)),(3,range(8,14)),(4,range(6,12))):
        for k,collar in product(ks,(True,False)):
            lanes=[(D-1-7*B)//2+7*x for x in range(B+1)]
            first=(D-1-2*k)//2
            layout=Geometry(lanes,[[(first+(j%2),k)] for j in range(B)],collar)
            wall,tubes=layout.make()
            assert len(wall)==(6 if collar else 4)*B*(k+1)
            assert len(tubes)==B*k
            union=set().union(*(t[0] for t in tubes))
            all_doors=set().union(*(t[1] for t in tubes))
            assert all(flood(next(iter(pen)),wall|all_doors)==pen for pen,_,_,_ in tubes)
            connected=flood((0,0),wall|all_doors)
            assert connected == BOARD-wall-union-all_doors
            print(int(collar),B,k,len(wall),len(tubes),len(connected),*[900-len(wall)-6*n for n in (10,15,20)],sep=',')
if __name__=='__main__':run()
