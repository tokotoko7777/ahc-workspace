"""Exhaustive two original top-edge roles within one fixed physical patch."""
from pathlib import Path
import sys,json,itertools as it
from collections import Counter
D=Path(__file__).resolve().parent
P=D.parent
sys.path.insert(0,str(P/'ahc008_terry_hunt_wall_audit'))
from analyze import terrain,cells,mask
from stochastic_lock_audit import ALL,B,flood,wave
W,_=terrain()[1]
catalog=[]
for line in (P/'ahc008_terry_small_rooms/catalog.tsv').read_text().splitlines():
    k,i,l,r,*u=map(int,line.split())
    if k==1:catalog.append((i,mask(u),mask(q for q in (l,r) if q>=0),
                               mask(q for q,b in ((l-1,l>=0),(r+1,r>=0)) if b)))
assert len(catalog)==76
patch=mask(r*30+c for r in (0,1)for c in range(5,14))
old=[z for z in catalog if z[0] in (1,2)]
retain=[z for z in catalog if z[0] not in (1,2)]
assert len(old)==2 and all((b|g)&~patch==0 for _,b,g,s in old)
assert (W&patch).bit_count()==4
r3=next(b for _,b,g,s in retain if b&B[69])
assert r3&B[69] and W&B[39] and wave(r3)&B[39]
aisles=mask(r*30+c for r in range(30)for c in (4,14,24))
outside=B[4]
black=mask(r*30+c for r in range(30)for c in range(30) if (r+c)%2==0)
stock_body_by_other=0;rest=0
for _,b,g,s in retain:stock_body_by_other|=b;rest|=g|s
config=sorted(cells(patch&~B[39]))
counts=Counter();best=None
for n in range(3):
    for combo in it.combinations(config,n):
        walls=(W&~patch)|mask(combo)|B[39]
        if walls&(stock_body_by_other|rest|aisles):continue
        floor=ALL^walls
        assert outside&floor
        for a,c in it.product(range(2),repeat=2):
            g1,s1,p1=30*a+5,30*a+4,30*a+6
            g2,s2,p2=30*c+13,30*c+14,30*c+12
            gates=B[g1]|B[g2]
            counts['projected']+=1
            if (gates|B[p1]|B[p2])&walls:continue
            u=flood(B[p1],floor^B[g1]);v=flood(B[p2],floor^B[g2])
            if u.bit_count()>4 or v.bit_count()>4:continue
            counts['both_small']+=1
            if (u|v)&~patch or u&(v|gates) or v&gates:continue
            if not (u&B[6]|u&B[7]|u&B[8]|u&B[9]|u&B[10]|u&B[11]|u&B[12]):continue
            if not (v&B[6]|v&B[7]|v&B[8]|v&B[9]|v&B[10]|v&B[11]|v&B[12]):continue
            uu=u&~wave(B[g1]);vv=v&~wave(B[g2])
            if not (uu&black and uu&~black and vv&black and vv&~black):continue
            counts['both_parity']+=1
            if flood(outside,floor)!=floor:continue
            checks=retain+[(1,u,B[g1],B[s1]),(2,v,B[g2],B[s2])]
            if any(flood(b&-b,floor^g)!=b or flood(outside,floor^g)!=floor&~(b|g)
                   or s&~flood(outside,floor^g) for _,b,g,s in checks):continue
            if flood(outside,floor^gates)!=floor&~(u|v|gates):continue
            blocked=0;allb=0
            for _,b,g,s in checks:blocked|=g;allb|=b
            avail=floor&~(blocked|allb)
            all_seen=0
            for x in cells(aisles):
                if not all_seen&B[x]:all_seen|=flood(B[x],avail)
            if all_seen!=avail:continue
            counts['verified']+=1
            z=(n,combo,a,c,list(cells(u)),list(cells(v)))
            if best is None or z<best:best=z
print(json.dumps(dict(stock_walls=(W&patch).bit_count(),counts=counts,witness=best)))
