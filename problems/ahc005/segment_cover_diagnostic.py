"""Inspect how many explicit long-segment obligations an incidence cover may omit."""
from pathlib import Path
import argparse

def sizes(p):
    n,r,c,*g=p.read_text().split()
    n,r,c=map(int,(n,r,c))
    ident=[[[-1]*n for _ in range(n)] for _ in range(2)]
    lengths=[[],[]]
    for t in range(2):
        for y in range(n):
            for x in range(n):
                rr,cc=(y,x) if not t else (x,y)
                if g[rr][cc]=='#':continue
                if x and ident[t][y][x-1]!=-1:i=ident[t][y][x-1]
                else:i=len(lengths[t]);lengths[t].append(0)
                ident[t][y][x]=i
                lengths[t][i]+=1
    edges=[(ident[0][y][x],ident[1][x][y]) for y in range(n) for x in range(n) if g[y][x]!='#']
    must=[{ident[0][r][c]},{ident[1][c][r]}]
    for x,y in edges:
        if lengths[0][x]==1:must[1].add(y)
        if lengths[1][y]==1:must[0].add(x)
    graph={}
    for x,y in edges:
        if x not in must[0] and y not in must[1]:graph.setdefault(x,[]).append(y)
    match={}
    def dfs(x,seen):
        for y in graph.get(x,[]):
            if y in seen:continue
            seen.add(y)
            if y not in match or dfs(match[y],seen):match[y]=x;return True
        return False
    for x in graph:dfs(x,set())
    longs=sum(q>1 for t in lengths for q in t)
    required=sum(lengths[t][i]>1 for t in range(2) for i in must[t])
    return longs,required,len(match)

if __name__=='__main__':
    ap=argparse.ArgumentParser()
    ap.add_argument('inputs',type=Path)
    ap.add_argument('ids',nargs='+',type=int)
    a=ap.parse_args()
    for i in a.ids:
        total,mandatory,optional=sizes(a.inputs/f'{i:04d}.txt')
        print(f'{i:04d} long={total} necessary={mandatory} minimum_additional={optional} removable={total-mandatory-optional}')
