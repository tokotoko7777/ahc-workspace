"""Independent integer replay of each upgrade and official score comparison."""
import argparse
import csv
import hashlib
import math
from pathlib import Path
import re
import subprocess
import tempfile
import time


def replay(data,out):
    n,l,t,apples,*fields=map(int,data.split())
    a=fields[:n]
    c=[fields[n+i*n:n+(i+1)*n] for i in range(l)]
    count=[[1]*n for _ in range(l)]
    power=[[0]*n for _ in range(l)]
    rows=out.splitlines()
    assert len(rows)==t
    active=0
    for line in rows:
        raw=list(map(int,line.split()))
        if raw!=[-1]:
            i,j=raw
            assert 0<=i<l and 0<=j<n
            power[i][j]+=1
            apples-=c[i][j]*power[i][j]
            assert apples>=0
            active+=1
        apples+=sum(a[j]*count[0][j]*power[0][j] for j in range(n))
        for i in range(1,l):
            for j in range(n):
                count[i-1][j]+=count[i][j]*power[i][j]
    return math.floor(100000*math.log2(apples)+0.5),apples,active


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','vis','base','macro','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,base,macro=(getattr(a,k).resolve() for k in ('gen','vis','base','macro'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','apples','upgrades','initial','iterations','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc058/macro-calendar-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(24)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                for label,binary in [('inherited',base),('ladder',macro)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,apples,active=replay(data,run.stdout)
                    detail=re.fullmatch(rb'iterations=(\d+) elapsed_ms=[\d.]+ initial=(\d+)\s*',run.stderr)
                    assert detail and elapsed<2,run.stderr
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}',judge.stdout
                    w.writerow((split,seed,label,score,apples,active,int(detail[2]),int(detail[1]),f'{elapsed:.6f}'))
                    file.flush()


if __name__=='__main__':
    main()
