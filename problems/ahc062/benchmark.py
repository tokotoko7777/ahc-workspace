"""Official and independent weighted king-path evaluation."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time


def replay(data,out):
    n,*a=map(int,data.split())
    size=n*n
    lines=out.splitlines()
    assert len(a)==size and len(lines)==size
    visited=set()
    last=None
    raw=0
    for k,line in enumerate(lines):
        y,x=map(int,line.split())
        assert 0<=y<n and 0<=x<n and (y,x) not in visited
        assert last is None or max(abs(y-last[0]),abs(x-last[1]))==1
        last=y,x
        visited.add(last)
        raw+=k*a[y*n+x]
    return (2*raw+size)//(2*size),raw


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','vis','twoopt','rotation','csv'):
        p.add_argument(k,type=Path)
    p.add_argument("--count",type=int,default=18)
    a=p.parse_args()
    gen,vis,twoopt,rotation=(getattr(a,k).resolve() for k in ('gen','vis','twoopt','rotation'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','raw','initial','trials','accepted','endpoint','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc062/path-rotate-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(a.count)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                initial=[]
                for label,binary in [('twoopt',twoopt),('rotation',rotation)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=3,check=True)
                    elapsed=time.monotonic()-begin
                    score,raw=replay(data,run.stdout)
                    detail=re.fullmatch(rb'init=(\d+) raw=(\d+) trials=(\d+) accepted=(\d+) endpoint=(\d+)\s*',run.stderr)
                    assert detail and int(detail[2])==raw and elapsed<3,run.stderr
                    initial.append(int(detail[1]))
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}',judge.stdout
                    w.writerow((split,seed,label,score,raw,initial[-1],*map(int,detail.groups()[2:]),f'{elapsed:.6f}'))
                    file.flush()
                assert initial[0]==initial[1]


if __name__=='__main__':
    main()
