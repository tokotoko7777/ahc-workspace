"""Verify circle consumption/creation, reflection and ordered visits."""
import argparse
from collections import Counter
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time


def replay(data,out):
    n,m,k,t,*tail=map(int,data.split())
    p=list(zip(tail[::2],tail[1::2],strict=True))
    assert len(p)==m+1
    goal=1
    y,x=p[0]
    lines=out.splitlines()
    assert k<=len(lines)<=k+t
    circles=Counter()
    for line in lines[:k]:
        a,b=map(int,line.split())
        assert 0<=a<n and 0<=b<n
        circles[a,b]+=1
    walking={b'U':(-1,0),b'D':(1,0),b'L':(0,-1),b'R':(0,1)}
    jumps=0
    for line in lines[k:]:
        c,*args=line.split()
        if c==b'J':
            a,b=map(int,args)
            assert 0<=a<n and 0<=b<n and circles[a,b]>0
            circles[a,b]-=1
            circles[y,x]+=1
            y,x=2*a-y,2*b-x
            jumps+=1
        else:
            assert not args
            a,b=walking[c]
            y,x=y+a,x+b
        assert 0<=y<n and 0<=x<n
        if goal<=m and (y,x)==p[goal]:
            goal+=1
    assert goal==m+1
    ops=len(lines)-k
    return m+t-ops+1,ops,jumps,t


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','vis','greedy','beam','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,greedy,beam=(getattr(a,k).resolve() for k in ('gen','vis','greedy','beam'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','ops','jumps','walk_budget','base','candidates','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc073/circle-state-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(28)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                starts=[]
                for label,binary in [('greedy',greedy),('beam',beam)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,ops,jumps,t=replay(data,run.stdout)
                    detail=re.fullmatch(rb'base=(\d+) moves=(\d+) candidates=(\d+)\s*',run.stderr)
                    assert detail and int(detail[2])==ops and elapsed<2,run.stderr
                    starts.append(int(detail[1]))
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}',judge.stdout
                    w.writerow((split,seed,label,score,ops,jumps,t,starts[-1],int(detail[3]),f'{elapsed:.6f}'))
                    file.flush()
                assert starts[0]==starts[1]


if __name__=='__main__':
    main()
