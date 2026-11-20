"""Judge an interactive forest plan with the official tester and independent replay."""
import argparse
from collections import deque
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time


def replay(data, out):
    lines=data.splitlines()
    n,ty,tx=map(int,lines[0].split())
    s=n//2
    t=n*ty+tx
    size=n*n
    board=[v!=ord('.') for line in lines[1:n+1] for v in line]
    raw=list(map(int,b' '.join(lines[n+1:]).split()))
    q=iter(n*y+x for y,x in zip(raw[::2],raw[1::2],strict=True))
    adj=[[] for _ in range(size)]
    step=[(-1,0),(1,0),(0,-1),(0,1)]
    for v in range(size):
        r,c=divmod(v,n)
        for dr,dc in step:
            if 0<=r+dr<n and 0<=c+dc<n:
                adj[v].append(n*(r+dr)+c+dc)

    def bfs(source,blocked):
        dist=[-1]*size
        dist[source]=0
        queue=deque([source])
        while queue:
            v=queue.popleft()
            for u in adj[v]:
                if dist[u]==-1 and not blocked[u]:
                    dist[u]=dist[v]+1
                    queue.append(u)
        return dist

    pos=s
    revealed={s}
    known_wall=[False]*size
    target=None
    target_dist=None
    moves=planted=0
    for line in out.splitlines():
        assert pos!=t
        k,*args=map(int,line.split())
        assert k>=0 and len(args)==2*k
        for y,x in zip(args[::2],args[1::2],strict=True):
            assert 0<=y<n and 0<=x<n
            v=y*n+x
            assert v not in revealed and not board[v] and v!=t
            board[v]=True
        planted+=k
        if k:
            assert bfs(s,board)[t]>=0
        changed=False
        r,c=divmod(pos,n)
        for dr,dc in step:
            y,x=r,c
            while 0<=y<n and 0<=x<n:
                v=y*n+x
                revealed.add(v)
                if board[v]:
                    if not known_wall[v]:
                        known_wall[v]=True
                        changed=True
                    break
                y+=dr
                x+=dc
        if t in revealed:
            if target!=t or changed:
                target=t
                target_dist=bfs(t,known_wall)
            assert target_dist[pos]>=0
        else:
            if target is not None and changed:
                target_dist=bfs(target,known_wall)
            if target is None or target in revealed or target_dist[pos]<0:
                reach=bfs(pos,known_wall)
                target=next(u for u in q if u not in revealed and reach[u]>=0)
                target_dist=bfs(target,known_wall)
        possibilities=[u for u in adj[pos] if target_dist[u]>=0]
        nxt=min(possibilities,key=target_dist.__getitem__)
        assert target_dist[nxt]<target_dist[pos] and not board[nxt]
        pos=nxt
        moves+=1
    assert pos==t
    return moves,planted,bfs(s,board)[t]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','tester','empty','shortest','induced','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,tester,empty,shortest,induced=(getattr(a,k).resolve()
                                      for k in ('gen','tester','empty','shortest','induced'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','planted','path','n','seconds'))
        work=Path(d)
        seeds=work/'seeds.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc054/induced-forest-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(24)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                data=(work/'in'/f'{i:04d}.txt').read_bytes()
                for label,binary in [('empty',empty),('shortest',shortest),('induced',induced)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([tester,binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,planted,path=replay(data,run.stdout)
                    m=re.fullmatch(rb'path=(\d+)\s+Score = (\d+)\s*',run.stderr)
                    assert m and int(m[2])==score and elapsed<2,run.stderr
                    assert score>=path and (label=='empty' and int(m[1])==0 or int(m[1])==path)
                    w.writerow((split,seed,label,score,planted,path,int(data.split()[0]),f'{elapsed:.6f}'))
                    file.flush()


if __name__=='__main__':
    main()
