"""Geometry-only exact-four AHC008 cage reuse of a diagonal dog fence."""
from itertools import product
from collections import Counter

D=30
DELTA=((-1,0),(1,0),(0,-1),(0,1))
board=set(product(range(D),repeat=2))

def nbs(s):
    return {q for x,y in s for dx,dy in DELTA if (q:=(x+dx,y+dy)) in board}

def norm(s):
    x0=min(x for x,y in s); y0=min(y for x,y in s)
    return tuple(sorted((x-x0,y-y0) for x,y in s))

def shapes():
    s={((0,0),)}
    for _ in range(3):
        s={norm(set(p)|{(x+dx,y+dy)}) for p in s for x,y in p
           for dx,dy in DELTA if (x+dx,y+dy) not in p}
    assert len(s)==19
    for p in sorted(s):
        xmax=max(x for x,y in p);ymax=max(y for x,y in p)
        for dx,dy in product(range(D-xmax),range(D-ymax)):
            a={ (x+dx,y+dy) for x,y in p }
            yield a, nbs(a)-a

def component(s,open_):
    seen={s}; q=[s]
    for p in q:
        for t in nbs({p}) & open_ - seen:
            seen.add(t);q.append(t)
    return seen

def run():
    motifs=list(shapes())
    print('depth,side,minimum_new_walls,number_min,number_with_3_or_less,number_with_4_or_less')
    for d in range(7,16):
        gate=(d//2,d-d//2)
        sep={(i,d-i) for i in range(d+1)}-{gate}
        for side in ('inside','outside'):
            counts=Counter()
            for pen,shell in motifs:
                if len(shell&sep)<2 or pen&sep or gate in pen|shell:
                    continue
                if side=='inside':
                    if not all(x+y<d for x,y in pen) or (0,0) in pen|shell: continue
                elif not all(x+y>d for x,y in pen):continue
                jobs=shell-sep
                # Keep ≥1 ready-to-build gate with an exterior stance and a
                # closure-safe on each parity; other jobs might be earlier.
                possible=[(g,guard) for g in jobs for guard in nbs({g})-pen-shell-sep
                          if {(x+y)%2 for x,y in pen-nbs({g})}=={0,1}]
                if not possible:continue
                rest=component((0,0), board-sep-shell-pen)
                if (D-1,D-1) not in rest or not any(guard in rest for g,guard in possible):
                    continue
                counts[len(jobs)]+=1
            k=min(counts, default=0)
            print(d,side,k,counts[k],sum(v for n,v in counts.items() if n<=3),sum(v for n,v in counts.items() if n<=4),sep=',')
if __name__=='__main__':run()
