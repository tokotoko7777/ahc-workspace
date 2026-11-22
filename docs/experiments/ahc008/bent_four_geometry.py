"""Preplanned irregular exact-four pen banks on staggered door rows (AHC008).

Each S pen fits between aisle columns at distance six and has a
one-row vertical offset between the two exterior guard stances.
All-closed connectivity is checked, not assumed.
"""
from tube_banks_geometry import BOARD,D,adj,flood,Geometry
from tube_banks_passive_ceiling import ceiling


class Bent:
    def __init__(self,B:int, first:int, offsets:bool=False):
        self.B=B; self.first=first; self.offsets=offsets
    def make(self):
        walls=set()
        tubes=[]
        for j in range(self.B):
            a=self.first+6*j
            for x in range(2,27,3):
                # Mirroring each other bank changes which aisle mouths share a row.
                f=lambda r,c:(r,a+(6-c if self.offsets and j%2 else c))
                pen={f(x,2),f(x,3),f(x+1,3),f(x+1,4)}
                gates={f(x,1),f(x+1,5)}
                stands={f(x,0),f(x+1,6)}
                ring={q for p in pen for q in adj(p)}-pen
                walls|=ring-gates
                safe=pen-{q for p in gates for q in adj(p)}
                assert len(safe)==2 and len({sum(p)%2 for p in safe})==2
                assert len(pen)==4 and set(pen).issubset(BOARD)
                tubes.append((pen,gates,stands,safe))
        doors=set().union(*(t[1] for t in tubes))
        inner=set().union(*(t[0] for t in tubes))
        assert len(inner)==4*len(tubes) and len(doors)==2*len(tubes)
        assert not(walls&inner or walls&doors)
        for pen,g,guard,safe in tubes:
            assert flood(next(iter(pen)),walls|doors)==pen
        people=flood((0,0),walls|doors)
        assert people==BOARD-walls-doors-inner
        assert all(g<=people for _,_,g,_ in tubes)
        return walls,tubes

if __name__=='__main__':
 print('type,banks,tubes,W,clean_if15,cow120,cow200,pig120,pig200,rabbit120,rabbit200')
 for typ,m,g in (
    ('flat',3,Geometry([4,11,18,25],[[(2,12)]]*3,False)),
    ('bent',3,Bent(3,5)), ('mirror',3,Bent(3,5,True)),
    ('bent',4,Bent(4,2)), ('mirror',4,Bent(4,2,True)),
 ):
    w,t=g.make()
    print(typ,m,len(t),len(w),810-len(w),*(f'{p:.6f}' for p in ceiling(g)),sep=',')
