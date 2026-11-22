"""Prebuilt mixed still-open and one-ended tube feasibility, exact on its stated model.

Passive transitions and stationary-exterior-target dog rules follow official v3.
Staffing is a model, not a live controller: pair/single readiness independent
at duty-epoch refresh, held for either 1 or 3 turns. Each duty state is exact;
arrival statistics do not simulate builders, other pets or adaptive targets.
"""
from collections import deque
import csv
import itertools
import sys
import numpy as np

D=30
V=D*D
BOARD=set(range(V))
NB=[tuple(r*D+c for r,c in ((p//D-1,p%D),(p//D+1,p%D),(p//D,p%D-1),(p//D,p%D+1))
          if 0<=r<D and 0<=c<D) for p in range(V)]


def layout(banks, orientation):
    k=12 if banks==3 else 13
    stride,shift=7,4 if banks==3 else 8
    lanes=[shift+stride*i for i in range(banks+1)]
    wall=set()
    tubes=[]
    for j,(a,b) in enumerate(zip(lanes,lanes[1:])):
        for r in range(2,2+2*k+1,2):
            wall.update(r*D+c for c in range(a+2,b-1))
        for i,r in enumerate(range(3,2+2*k,2)):
            pen={r*D+c for c in range(a+2,b-1)}
            gates=[r*D+a+1,r*D+b-1]
            cap=None
            if (i+j)%3==1 and orientation is not None:
                cap=gates[0 if orientation=='left' else 1 if orientation=='right' else ((i//3+j)%2)]
            tubes.append((pen,gates,cap))
    assert len(wall)==4*banks*(k+1)
    caps={p for _,_,p in tubes if p is not None}
    assert len(caps)==sum(p is not None for _,_,p in tubes)
    wall=wall|caps
    safe_pair=set()
    safe_single=set()
    for body, gates, cap in tubes:
        open_gates=[q for q in gates if q!=cap]
        adjacent={q for p in open_gates for q in NB[p]}
        safe=body-adjacent
        assert len(safe)==(3 if cap is not None else 2)
        (safe_single if cap is not None else safe_pair).update(safe)
    assert not safe_pair&safe_single and not (safe_pair|safe_single)&wall
    body_cap=set().union(*(p for p,g,c in tubes if c is not None))
    homes=tuple(sorted({lanes[0],lanes[-1],29*D+lanes[1],29*D+lanes[-1],14*D+lanes[1]}))
    all_gates=set().union(*(set(g) for _,g,_ in tubes))
    def component(p, walls):
        seen={p}; work=[p]
        for x in work:
            for y in NB[x]:
                if y not in walls and y not in seen:seen.add(y);work.append(y)
        return seen
    fully_shut=wall|all_gates
    assert all(component(min(body),fully_shut)==body for body,_,_ in tubes)
    cut_bodies=set().union(*(body for body,_,_ in tubes))
    assert component(homes[0],fully_shut)==BOARD-fully_shut-cut_bodies
    return wall,caps,body_cap,safe_pair,safe_single,homes,tubes


class Motion:
    def __init__(self, wall, homes):
        self.wall=wall
        self.valid=sorted(BOARD-wall)
        self.home=homes
        self.graph={p:tuple(q for q in NB[p] if q not in wall) for p in self.valid}
        self._init_uniform()
        assert len(self.dist(homes[0]))==len(self.valid), 'unclosed exterior must stay connected'

    def dist(self, p):
        out={p:0}; todo=deque([p])
        while todo:
            u=todo.popleft()
            for v in self.graph[u]:
                if v not in out:
                    out[v]=out[u]+1;todo.append(v)
        return out

    def _init_uniform(self):
        ed=[(p,q,1./len(self.graph[p])) for p in self.valid for q in self.graph[p]]
        assert all(self.graph[p] for p in self.valid)
        self.uniform=self.pack(ed)

    def pack(self, ed):
        x,y,z=zip(*ed)
        return np.array(x,dtype=np.intp),np.array(y,dtype=np.intp),np.array(z)

    def dog(self):
        H=len(self.home); n=V*(H+1)
        dist=[self.dist(p) for p in self.home]
        ed=[]
        for p in self.valid:
            for flag in range(H+1):
                choices=[i for i,tar in enumerate(self.home) if p!=tar] if flag==H or p==self.home[flag] else [flag]
                # All valid cells in this prebuilt-open graph reach every fixed exterior human.
                for i in choices:
                    tar=self.home[i]; dm=dist[i]
                    directed=[q for q in self.graph[p] if dm[q]<dm[p]]
                    assert directed
                    for q in directed:
                        for r in self.graph[q]:
                            new=H if q==tar or r==tar else i
                            ed.append((p+flag*V,r+new*V,1./(len(choices)*len(directed)*len(self.graph[q]))))
        movers=self.pack(ed)
        mass=np.bincount(movers[0],weights=movers[2],minlength=n)
        for k in range(H+1):
            assert np.max(np.abs(mass[k*V+np.array(self.valid)]-1.))<1e-9
        return movers,n


def staff_probability(p, q, hp=True, hs=True):
    # Enumerate independent pair/single duty switches at each epoch.
    a=((1.,False),) if not hp else ((1-p,False),(p,True))
    b=((1.,False),) if not hs else ((1-q,False),(q,True))
    return [(w*v,x,y) for (w,x),(v,y) in itertools.product(a,b) if w*v]


def propagate(mech, n, pace, start, pair, single, terms, p, q, epoch):
    ori,dest,weights=mech
    state=np.zeros(n)
    off=n-V if n>V else 0
    state[np.array(sorted(start),dtype=np.intp)+off]=1./len(start)
    ways=staff_probability(p,q,bool(pair),bool(single))
    both=[]
    for w,x,y in ways:
        a=np.array([j+k for j in pair for k in range(0,n,V)] if x else [],dtype=np.intp)
        b=np.array([j+k for j in single for k in range(0,n,V)] if y else [],dtype=np.intp)
        both.append((w,a,b))
    at=0; results={}
    captured_pair=captured_single=0.
    # Each block fixes availability for its whole duty epoch. Record terms
    # at identical state clock before the next move, after current closure test.
    while at<=max(terms):
        stop=min(at+epoch,max(terms)+1)
        merged=np.zeros(n)
        accum={t:[0.,0.] for t in terms if at<=t<stop}
        epA=epB=0.
        for w,a,b in both:
            st=state.copy(); ca=cb=0.
            for t in range(at,stop):
                ca+=float(st[a].sum()); cb+=float(st[b].sum())
                st[a]=0;st[b]=0
                if t in accum:
                    accum[t][0]+=w*ca;accum[t][1]+=w*cb
                for _ in range(pace):
                    st=np.bincount(dest,weights=st[ori]*weights,minlength=n)
            merged+=w*st;epA+=w*ca;epB+=w*cb
        for t,(x,y) in accum.items():
            results[t]=(captured_pair+x,captured_single+y)
        captured_pair+=epA;captured_single+=epB
        state=merged;at=stop
    assert abs(captured_pair+captured_single+state.sum()-1)<1e-8
    return results


def create_rows():
    # Bank three's empirical per-tube legal staffed ~0.11 is an anchor,
    # not a universal population parameter; plausible range is 0.07–0.18.
    duties=((.07,.20),(.11,.20),(.11,.33),(.11,.50),(.11,.75),(.11,1.),(.18,.33),(.18,.50))
    terms=(60,120,160)
    for banks in (2,3):
        base=layout(banks,None)
        wm,cp,inside,sp,ss,h,tubes=base
        motion0=Motion(wm,h)
        dog0,n0=motion0.dog()
        for orientation in ('left','right','checker'):
            hybrid=layout(banks,orientation)
            wall,caps,inside,s_pair,s_single,homes,tubes=hybrid
            motion1=Motion(wall,homes)
            dog1,n1=motion1.dog()
            starts=set(motion1.valid)
            # In capped branches no legal sample point is reachable from
            # outside by directed + random substeps to fixed exterior targets.
            modes=[
                ('cow','matched_uniform',1,starts,motion0.uniform,motion1.uniform,V,V),
                ('pig','matched_uniform',2,starts,motion0.uniform,motion1.uniform,V,V),
                ('rabbit','matched_uniform',3,starts,motion0.uniform,motion1.uniform,V,V),
                ('dog','outside_capped_bodies',1,starts-inside,dog0,dog1,n0,n1),
                ('dog','already_in_capped_body',1,inside,dog0,dog1,n0,n1),
            ]
            for p,q in duties:
                for epoch in (1,3):
                    for species,where,pace,start,m0,m1,z0,z1 in modes:
                        x=propagate(m0,z0,pace,start,sp,ss,terms,p,q,epoch)
                        y=propagate(m1,z1,pace,start,s_pair,s_single,terms,p,q,epoch)
                        for t in terms:
                            u,v=y[t]
                            if species=='dog' and where=='outside_capped_bodies': assert v<1e-13
                            yield dict(banks=banks,orientation=orientation,cap_walls=len(caps),species=species,
                                       start=where,epoch=epoch,pair_duty=p,single_duty=q,turn=t,
                                       baseline=round(sum(x[t]),9),hybrid=round(u+v,9),
                                       paired=round(u,9),single=round(v,9),delta=round(u+v-sum(x[t]),9))


if __name__=='__main__':
    wr=None
    for row in create_rows():
        if wr is None:wr=csv.DictWriter(sys.stdout,list(row),lineterminator='\n');wr.writeheader()
        wr.writerow(row)
