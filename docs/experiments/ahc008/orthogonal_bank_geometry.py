"""AHC008 compact cages split into disjoint rectangles with perpendicular travel.
The idealized passive arrival metric recomputes walls and ignores staffing/setup.
No simulator data, opponent code, or per-case tuning is used."""
from tube_banks_geometry import BOARD,D,adj,flood,Geometry
from tube_banks_passive_ceiling import ceiling

class Orthogonal:
    def __init__(self,top=7,lower=13,base=20):
        self.top,self.lower,self.base=top,lower,base
    def make(self):
        wall=set()
        tubes=[]
        def add(a,b,start,count,rot):
            assert b-a==7
            f=lambda r,x:(x,r) if rot else (r,x)
            for i in range(count):
                r=start+2*i
                body={f(r,a+k) for k in range(2,6)}
                l,g,k,h=[f(r,a+z) for z in (0,1,6,7)]
                wall.update(f(r+d,a+k) for d in (-1,1) for k in range(2,6))
                tubes.append((body,{g,k},{l,h},{f(r,a+3),f(r,a+4)}))
        for n in range(3):
            add(4+7*n,11+7*n,2,self.top,False)
        add(self.base,self.base+7,3,self.lower,True)
        body=set().union(*(x[0] for x in tubes))
        gates=set().union(*(x[1] for x in tubes))
        stance=set().union(*(x[2] for x in tubes))
        assert len(body)==4*len(tubes)
        assert len(gates)==2*len(tubes)
        assert not wall&(body|gates|stance)
        assert not stance&(gates|body)
        assert (wall|body|gates|stance)<=BOARD
        avail=BOARD-wall-gates-body
        assert flood((0,0),wall|gates)==avail
        for x,g,s,safe in tubes:
            assert flood(next(iter(x)),wall|gates)==x
            assert len(safe)==2 and len({sum(p)%2 for p in safe})==2
        return wall,tubes

if __name__=='__main__':
    print("variant,tubes,braces,clean_after_15,cow120,cow200,pig120,pig200,rabbit120,rabbit200")
    candidates=[('baseline',Geometry([4,11,18,25],[[(2,12)]]*3,False))]
    candidates += [(f'x{h}-{r}-{k}',Orthogonal(h,k,r)) for h in (5,6,7) for r in (14,15,17,20,22) for k in (12,13)]
    for label,geom in candidates:
        try: w,t=geom.make()
        except AssertionError: continue
        print(label,len(t),len(w),810-len(w),*(f'{p:.6f}' for p in ceiling(geom)),sep=',')
