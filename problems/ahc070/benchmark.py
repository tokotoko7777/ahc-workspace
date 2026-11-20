"""Check official scorer with independent placement-time and distance-ring replay."""
import argparse
import csv
import hashlib
from math import isqrt
from pathlib import Path
import re
import subprocess
import tempfile
import time


def replay(data,out):
    n,m,*pairs=map(int,data.split())
    targets=list(zip(pairs[::2],pairs[1::2],strict=True))
    rows=[list(map(int,line.split())) for line in out.splitlines()]
    size=n*n
    assert len(targets)==size and len(rows)==size+m
    steps=rows[:m]
    assert all(len(row)==2 and all(0<=v<n for v in row) for row in steps)
    first=[[size]*n for _ in range(n)]
    y=x=0
    hits=0
    for t,item in enumerate(rows[m:]):
        assert len(item)==1 and 0<=item[0]<m
        dy,dx=steps[item[0]]
        y,x=(y+dy)%n,(x+dx)%n
        if first[y][x]==size:
            first[y][x]=t
            hits+=1
    danger=0
    for t,(a,b) in enumerate(targets):
        for dist in range(2*n-1):
            found=False
            for y in range(max(0,a-dist),min(n,a+dist+1)):
                rem=dist-abs(a-y)
                p,q=b-rem,b+rem
                if (0<=p<n and first[y][p]<=t) or (0<=q<n and first[y][q]<=t):
                    found=True
                    break
            if found:
                danger+=isqrt(dist*dist*(t+1))
                break
        else:
            raise AssertionError('no placed source')
    value=10**6*n**3
    return (2*value+danger+1)//(2*(danger+1)),danger,hits


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','vis','near','spread','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,near,spread=(getattr(a,k).resolve() for k in ('gen','vis','near','spread'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as f:
        w=csv.writer(f,lineterminator='\n')
        w.writerow(('split','seed','arm','score','danger','coverage','frozen','trials','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc070/spread-translations-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(24)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                starts=[]
                for label,binary in [('near',near),('spread',spread)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,danger,coverage=replay(data,run.stdout)
                    detail=re.fullmatch(rb'frozen=(\d+) danger=(\d+) trials=(\d+)\s*',run.stderr)
                    assert detail and int(detail[2])==danger and int(detail[3])==8 and elapsed<2,run.stderr
                    starts.append(int(detail[1]))
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}',judge.stdout
                    w.writerow((split,seed,label,score,danger,coverage,starts[-1],int(detail[3]),f'{elapsed:.6f}'))
                    f.flush()
                assert starts[0]==starts[1]


if __name__=='__main__':
    main()
