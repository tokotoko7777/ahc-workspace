"""Independent macro expansion and robot replay against official vis."""
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
    n,m,t=map(int,lines[0].split())
    v=lines[1:1+n]
    h=lines[1+n:2*n]
    board={}
    basket=[]
    for i,line in enumerate(lines[2*n:]):
        a,b,c,d=map(int,line.split())
        board[a,b]=i
        basket.append((c,d))
    assert len(basket)==m
    commands=out.splitlines()
    assert len(commands)<=t
    y=x=direction=0
    hand=None
    committed=[]
    recording=None
    basic=0
    for c in commands:
        if c==b'M':
            if recording is None:
                recording=[]
            else:
                committed=recording
                recording=None
            continue
        assert c in (b'F',b'R',b'L',b'S',b'P')
        seq=committed if c==b'P' else [c]
        for step in seq:
            basic+=1
            assert basic<=t
            if recording is not None:
                recording.append(step)
            if step==b'R':
                direction=(direction+1)%4
            elif step==b'L':
                direction=(direction-1)%4
            elif step==b'F':
                if direction==0 and x+1<n and v[y][x]==ord('0'):
                    x+=1
                elif direction==1 and y+1<n and h[y][x]==ord('0'):
                    y+=1
                elif direction==2 and x>0 and v[y][x-1]==ord('0'):
                    x-=1
                elif direction==3 and y>0 and h[y-1][x]==ord('0'):
                    y-=1
            else:
                hand,board[y,x]=board.get((y,x)),hand
    completed=sum(board.get(q)==i for i,q in enumerate(basket))
    assert completed==m and hand is None
    return len(commands),basic,m,sum(c==b'P' for c in commands)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','vis','trace','reroute','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,trace,reroute=(getattr(a,k).resolve() for k in ('gen','vis','trace','reroute'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','expanded','balls','plays','literal','frozen','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc066/pose-macro-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(28)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                starts=[]
                for label,binary in [('trace',trace),('reroute',reroute)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,expanded,balls,plays=replay(data,run.stdout)
                    detail=re.fullmatch(rb'literal=(\d+) base=(\d+) score=(\d+) expanded=(\d+)\s*',run.stderr)
                    assert detail and (int(detail[3]),int(detail[4]))==(score,expanded) and elapsed<2,run.stderr
                    starts.append((int(detail[1]),int(detail[2])))
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}',judge.stdout
                    w.writerow((split,seed,label,score,expanded,balls,plays,*starts[-1],f'{elapsed:.6f}'))
                    file.flush()
                assert starts[0]==starts[1]


if __name__=='__main__':
    main()
