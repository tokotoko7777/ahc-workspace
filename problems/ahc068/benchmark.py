"""Replay wall-free half-swaps and compare complete sorting to official vis."""
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
    tokens=data.split()
    n=int(tokens[0])
    size=n*n
    board=list(map(int,tokens[1:1+size]))
    vertical=tokens[1+size:1+size+n]
    horizontal=tokens[1+size+n:]
    assert len(horizontal)==n-1
    operations=out.splitlines()
    assert 0<len(operations)<=100000
    area=0
    for line in operations:
        d,*fields=line.split()
        r,c,h,w=map(int,fields)
        assert d in (b'V',b'H') and 0<=r<r+h<=n and 0<=c<c+w<=n
        assert (h if d==b'V' else w)%2==0
        assert all(49 not in vertical[y][c:c+w-1] for y in range(r,r+h))
        assert all(49 not in horizontal[y][c:c+w] for y in range(r,r+h-1))
        k=h//2*n if d==b'V' else w//2
        for y in range(r,r+(h//2 if d==b'V' else h)):
            for x in range(c,c+(w//2 if d==b'H' else w)):
                a=y*n+x
                board[a],board[a+k]=board[a+k],board[a]
        area+=w*h
    assert board==list(range(size))
    score=size+math.floor(1_000_000*math.log2(100000/len(operations))+0.5)
    return score,len(operations),area


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','vis','adjacent','strips','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,adjacent,strips=(getattr(a,k).resolve() for k in ('gen','vis','adjacent','strips'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','ops','area','base','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc068/active-strip-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(26)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                starts=[]
                for label,binary in [('adjacent',adjacent),('strips',strips)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,ops,area=replay(data,run.stdout)
                    detail=re.fullmatch(rb'base=(\d+) turns=(\d+)\s*',run.stderr)
                    assert detail and int(detail[2])==ops and elapsed<2,run.stderr
                    starts.append(int(detail[1]))
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}',judge.stdout
                    w.writerow((split,seed,label,score,ops,area,starts[-1],f'{elapsed:.6f}'))
                    file.flush()
                assert starts[0]==starts[1]


if __name__=='__main__':
    main()
