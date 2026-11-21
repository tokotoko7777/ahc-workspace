#!/usr/bin/env python3
"""Generate legal graphs outside the official distribution for robustness tests."""
import math
from pathlib import Path
import random
import sys

N,M=400,1995
out=Path(sys.argv[1])
out.mkdir(exist_ok=True,parents=True)
r=random.Random(7826171)
for case in range(36):
    xy=[]
    while len(xy)<N:
        p=r.randrange(801),r.randrange(801)
        if all((p[0]-q[0])**2+(p[1]-q[1])**2>25 for q in xy):
            xy.append(p)
    blocks=20 if case%3 else 1
    size=N//blocks
    links=set()
    # Each block is connected; exactly one edge joins successive blocks.
    bridges=set()
    for k in range(blocks):
        base=k*size
        edges={(base+r.randrange(j),base+j) for j in range(1,size)}
        links.update(edges)
        if k:
            bridges.add((base-size+r.randrange(size),base+r.randrange(size)))
    links.update(bridges)
    while len(links)<M:
        base=r.randrange(blocks)*size
        u,v=sorted(r.sample(range(base,base+size),2))
        links.add((u,v))
    edges=sorted(links)
    r.shuffle(edges)
    if case%6==1:
        edges.sort(key=bridges.__contains__) # bridge costs are revealed last
    if case%6==2:
        edges.sort(key=bridges.__contains__,reverse=True)
    geo=[int(math.hypot(xy[u][0]-xy[v][0],xy[u][1]-xy[v][1])+0.5) for u,v in edges]
    w=[r.randrange(d,3*d+1) if (case%6<4) else (d if case%6==4 else 3*d) for d in geo]
    (out/f"{case:04}.txt").write_text("".join(f"{a} {b}\n" for a,b in xy+edges)+"".join(f"{l}\n" for l in w))
