"""Replay the grid machine and compare to official visualizer."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time


def replay(data,out):
    lines=data.splitlines()
    n,k,t=map(int,lines[0].split())
    v,h=lines[1:1+n],lines[1+n:2*n]
    goals=[tuple(map(int,line.split())) for line in lines[2*n:]]
    assert len(goals)==k
    rules={}
    rows=out.splitlines()
    c,q,m=map(int,rows[0].split())
    assert 0<c<=n**4 and 0<q<=n**4 and m<=t and len(rows)==n+1+m
    color=[list(map(int,line.split())) for line in rows[1:1+n]]
    assert all(len(row)==n and all(0<=x<c for x in row) for row in color)
    for row in rows[1+n:]:
        ci,qi,ai,si,d=row.split()
        ci,qi,ai,si=map(int,(ci,qi,ai,si))
        assert 0<=ci<c and 0<=ai<c and 0<=qi<q and 0<=si<q and d in (b'U',b'D',b'L',b'R',b'S')
        assert (ci,qi) not in rules
        rules[ci,qi]=(ai,si,d)
    y,x=goals[0]
    state=0
    visited=1
    step=0
    while step<t and visited<k:
        rule=rules.get((color[y][x],state))
        if rule is None:
            break
        color[y][x],state,d=rule
        if d==b'U' and y>0 and h[y-1][x]==ord('0'):
            y-=1
        elif d==b'D' and y+1<n and h[y][x]==ord('0'):
            y+=1
        elif d==b'L' and x>0 and v[y][x-1]==ord('0'):
            x-=1
        elif d==b'R' and x+1<n and v[y][x]==ord('0'):
            x+=1
        step+=1
        if (y,x)==goals[visited]:
            visited+=1
    assert k==visited
    return c+q,c,q,m,step


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','vis','base','allocate','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,base,allocate=(getattr(a,k).resolve() for k in ('gen','vis','base','allocate'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','colors','states','rules','steps','base','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc056/reverse-rule-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(32)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                records=[]
                for label,binary in [('scheduled',base),('allocated',allocate)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,c,q,m,step=replay(data,run.stdout)
                    detail=re.fullmatch(rb'base=(\d+) cost=(\d+) steps=(\d+)\s*',run.stderr)
                    assert detail and (int(detail[2]),int(detail[3]))==(score,step) and elapsed<2,run.stderr
                    records.append((int(detail[1]),step))
                    ans.write_bytes(run.stdout)
                    official=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert official.stdout.strip()==f'Score = {score}',official.stdout
                    w.writerow((split,seed,label,score,c,q,m,step,records[-1][0],f'{elapsed:.6f}'))
                    file.flush()
                assert records[0]==records[1]


if __name__=='__main__':
    main()
