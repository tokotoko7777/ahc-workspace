from itertools import combinations
from collections import Counter
from collections import deque
import json

def ns(p):
    x,y=p
    return [(x-1,y),(x+1,y),(x,y-1),(x,y+1)]

def canonical(s):
    a,b=min(x for x,y in s),min(y for x,y in s)
    return tuple(sorted((x-a,y-b) for x,y in s))

def flood(p, ok):
    seen={p}; q=deque([p])
    while q:
        for v in ns(q.popleft()):
            if v not in seen and ok(v):
                seen.add(v);q.append(v)
    return seen

def advance(pos, legal):
    out=set()
    for p in pos:
        steps=[v for v in ns(p) if legal(v)]
        out.update(steps or [p])
    return out

def count(n, shapes):
    c=Counter(orientations=len(shapes))
    witness={}
    for body_tuple in sorted(shapes):
        body=set(body_tuple)
        ring={v for p in body for v in ns(p)}-body
        for g,h in combinations(sorted(ring),2):
            common=set(ns(g))&set(ns(h))-body-ring
            if not common: continue
            walls=ring-{g,h}
            exterior=lambda p: p not in walls|{g,h} and -4<=p[0]<=8 and -4<=p[1]<=8
            ext=flood((-4,-4),exterior)
            if not(common&ext):continue
            if not(set(ns(g))&ext and set(ns(h))&ext):continue
            c["geometries"]+=1
            for k in (1,2,3):
                safe=set()
                for first,last in ((g,h),(h,g)):
                    def legal(v):
                        return v not in walls|{first}
                    for p in body:
                        if p in ns(first): continue
                        end={p}
                        for _ in range(k):end=advance(end,legal)
                        if end <= body and not(end&set(ns(last))):
                            safe.add(p)
                            witness.setdefault(str(k),[body_tuple,first,last,p,sorted(common&ext)])
                if safe:
                    c[f"stride{k}_any"]+=1
                    c[f"stride{k}_starts"]+=len(safe)
                if len({(x+y)%2 for x,y in safe})==2:
                    c[f"stride{k}_both_parities"]+=1
    return {"counts":c,"examples":witness}

def run():
    shapes={((0,0),)}
    out={}
    for n in range(1,5):
        out[n]=count(n,shapes)
        shapes={canonical(set(s)|{v}) for s in shapes for p in s for v in ns(p) if v not in s}
    assert [x["counts"]["orientations"] for x in out.values()]==[1,2,6,19]
    assert all(not x["counts"]["stride2_both_parities"] and not x["counts"]["stride3_any"] for x in out.values())
    return out
if __name__=="__main__":
    print(json.dumps(run(),indent=2))
