"""Enumerate area 5 and 6 with the previous audit's exact semantics."""
from collections import defaultdict
from itertools import combinations
import importlib.util
import json
from pathlib import Path

parent=Path(__file__).resolve().parent.parent
path=parent / "terry-one-guard-dual-geometry" / "audit.py"
if not path.is_file():
    path=parent / "ahc008_terry_one_guard_dual_geometry" / "audit.py"
spec=importlib.util.spec_from_file_location("previous_audit",path)
m=importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

def raw_canonical(body,pair):
    a,b=min(x for x,y in body),min(y for x,y in body)
    return (tuple(sorted((x-a,y-b) for x,y in body)),
            tuple(sorted((x-a,y-b) for x,y in pair)))

def dihedral(body,pair):
    return min(raw_canonical([(i*x,j*y) for x,y in b],[(i*x,j*y) for x,y in p])
               for b,p in ((body,pair),([(y,x) for x,y in body],[(y,x) for x,y in pair]))
               for i in (-1,1) for j in (-1,1))

def safe_starts(body,walls,g,h,k):
    out=set()
    for first,last in ((g,h),(h,g)):
        for p in body - set(m.ns(first)):
            cur={p}
            for _ in range(k):
                cur=m.advance(cur, lambda v: v not in walls and v != first)
            if cur <= body and not(cur & set(m.ns(last))):
                out.add(p)
    return out

def count(shapes):
    all_pairs=0
    jointly=defaultdict(dict)
    for s in sorted(shapes):
        body=set(s)
        ring={v for p in body for v in m.ns(p)}-body
        for g,h in combinations(sorted(ring),2):
            common=set(m.ns(g)) & set(m.ns(h)) - body - ring
            if not common: continue
            walls=ring-{g,h}
            exterior=lambda p: p not in walls | {g,h} and -4 <= p[0] <= 10 and -4 <= p[1] <= 10
            exterior_cells=m.flood((-4,-4),exterior)
            if not common & exterior_cells: continue
            all_pairs+=1
            pigs=safe_starts(body,walls,g,h,2)
            if len({(x+y)%2 for x,y in pigs}) != 2: continue
            rabbits=safe_starts(body,walls,g,h,3)
            if not rabbits: continue
            key=dihedral(body,(g,h))
            entry=jointly[len(walls)].setdefault(key,{
                "body":s,"gates":(g,h),"walls":sorted(walls),
                "stances":sorted(common & exterior_cells),
                "cow":sorted(safe_starts(body,walls,g,h,1)),
                "pig":sorted(pigs),"rabbit":sorted(rabbits),"orientations":0
            })
            entry["orientations"]+=1
    return {"orientations":len(shapes),"all_pairs":all_pairs,
            "qualified_by_walls":{k:{"classes":len(v),"orientations":sum(e["orientations"] for e in v.values())}
                                 for k,v in sorted(jointly.items())},
            "minimum_wall_layouts":list(jointly[min(jointly)].values()) if jointly else []}
def run():
    shapes={((0,0),)}
    res={}
    for n in range(1,7):
        if n in (5,6):res[n]=count(shapes)
        if n<6:
            shapes={m.canonical(set(s)|{v}) for s in shapes for p in s for v in m.ns(p) if v not in s}
    assert [res[n]["orientations"] for n in (5,6)]==[63,216]
    return res
if __name__=="__main__":
    print(json.dumps(run(),indent=2))
