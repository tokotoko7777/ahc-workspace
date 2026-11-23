from pathlib import Path
from collections import Counter
from fractions import Fraction
from random import Random
import sys, json, statistics
p=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(p/'ahc008_terry_hunt_wall_audit'))
from analyze import terrain,cells,mask
from stochastic_lock_audit import ALL,B,flood

def neighbors(i):
    r,c=divmod(i,30)
    return [x for x,ok in ((i-1,c>0),(i+1,c<29),(i-30,r>0),(i+30,r<29)) if ok]

def prob(nbs,stride,targets,horizons=(10,25,50,100)):
    scores={}
    v=[float(i in targets) for i in range(len(nbs))]
    for h in range(1,max(horizons)+1):
        for _ in range(stride):
            v=[sum(v[q] for q in options)/len(options) for options in nbs]
        v=[1. if i in targets else x for i,x in enumerate(v)]
        if h in horizons:scores[h]=v
    return scores

def unreachable(pos,nbs,targets,stride):
    reachable=set(targets); frontier=set(targets)
    while frontier:
        for _ in range(stride): frontier={q for i in frontier for q in nbs[i]}
        frontier-=reachable;reachable|=frontier
    return len(pos-reachable)

def verify():
    # Four-cell straight segment and endpoints; two steps preserve parity.
    path=[[1],[0,2],[1,3],[2]]
    assert prob(path,1,{0},(1,2,3))[2][2]==.25
    assert prob(path,2,{0},(1,2))[1]==[1,0,.25,0]
    assert unreachable(set(range(4)),path,{0},2)==2
    assert unreachable(set(range(4)),path,{0},3)==0
    rng=Random(121827)
    from functools import lru_cache
    for _ in range(100):
        n=rng.randrange(2,9)
        a=[set() for _ in range(n)]
        for i in range(n):
            for j in range(i):
                if rng.randrange(3)==0:a[i].add(j);a[j].add(i)
        a=[sorted(s) or [i] for i,s in enumerate(a)]
        goal={i for i in range(n) if rng.randrange(3)==0}
        for k in (1,2,3):
            result=prob(a,k,goal,(1,2))
            @lru_cache(None)
            def oracle(x,remaining):
                if remaining%k==0 and x in goal:return Fraction(1)
                if remaining==0:return Fraction(0)
                return sum((oracle(q,remaining-1) for q in a[x]),Fraction())/len(a[x])
            for h,values in result.items():
                assert all(abs(v-float(oracle(i,k*h)))<1e-14 for i,v in enumerate(values))

def stats(pos,probs):
    a=sorted(probs[i] for i in pos)
    return [round(statistics.fmean(a),8),round(a[(len(a)-1)//10],8)]
def main():
    verify()
    catalog={i:[] for i in range(5)}
    for z in (p/'ahc008_terry_small_rooms/catalog.tsv').read_text().splitlines():
        m,i,l,r,*a=map(int,z.split())
        catalog[m].append((i,set(a),[q for q in (l,r) if q>=0]))
    result={}
    for m,(x,g) in enumerate(terrain()):
        pos=set(cells(ALL^x))
        nbs=[[q for q in neighbors(i) if q in pos] or [i] for i in range(900)]
        choices={"all":set(),"narrow":set()}
        verified=Counter()
        for i,body,gates in catalog[m]:
            locked=(ALL^x)&~mask(gates)
            exact=set(cells(flood(B[min(body)],locked)))
            if exact!=body:
                assert m==1 # Only Map1 has nominal/physical discrepancies audited independently.
                verified['rejected_nominal']+=1
                # Reconstruct its effective physical room, only if all nominal cells
                # still map to it and the exterior remains connected and large.
                if not(body<=exact and 1<=len(exact)<=15):continue
                assert not(exact&set(gates))
                body=exact;verified['effective_reconstructed']+=1
            verified['valid_rooms']+=1
            safe=body-{q for g in gates for q in neighbors(g)}
            choices["all"].update(safe)
            if len(body)<=4: choices["narrow"].update(safe);verified['narrow_rooms']+=1
        split={str(parity):{i for i in pos if sum(divmod(i,30))%2==parity} for parity in (0,1)}
        result[m]={"open_cells":len(pos),"rooms":dict(verified),"eligible":{c:len(v) for c,v in choices.items()},"stride":{}}
        for k in (1,2,3):
            result[m]["stride"][k]=r={}
            for scenario,target in choices.items():
                hp=prob(nbs,k,target)
                r[scenario]={"unreachable":unreachable(pos,nbs,target,k),"scores":{
                    h:{"all":stats(pos,v),**{par:stats(ps,v) for par,ps in split.items()}}for h,v in hp.items()}}
    print(json.dumps(result,separators=(",",":")))
if __name__=="__main__":main()
