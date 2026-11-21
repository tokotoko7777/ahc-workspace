from partial import *
import subprocess,time
from random import Random
def best_similarity(b,known):
    out=0; known=known[:20]; cells=[(r,c,s) for r,row in enumerate(b) for c,s in enumerate(row) if s!='.']
    for d in [0,1]:
        q=known if d==0 else list(map(''.join,zip(*known)))
        for r in range(20):
            for c in range(20):
                out=max(out,sum(q[(r+y)%20][(c+x)%20]==s for y,x,s in cells))
    return out,len(cells)
rng=Random(9187)
for L,M in [(6,650),(6,800),(7,450),(7,650)]:
    for j in range(4):
        b,w=make_case(L,M,6_030_501_213+L*120+j)
        o=subprocess.run(['./synth_v2'],input=f'{M} 1600 192\n'+'\n'.join(w)+'\n',capture_output=True,text=True,check=True,timeout=3)
        raw=o.stdout.splitlines()
        res=[]
        for i in range(0,len(raw),21):
            ss,rows,cols,st,lo,ex=map(int,raw[i].split())
            g=raw[i+1:i+21];a,k=best_similarity(g,b)
            res.append((ss,a,k,rows,cols,st,lo,ex))
        print(L,M,j,o.stderr.strip(),len(res),sorted(res,reverse=True)[:3],max(res,key=lambda z:z[1]) if res else None,flush=True)
