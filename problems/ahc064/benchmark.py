"""Official visualizer and independent track/capacity/commutation replay."""
import argparse
from collections import deque
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time


def replay(data,out):
    r,*cars=map(int,data.split())
    assert len(cars)==10*r
    dep=[cars[10*i:10*(i+1)] for i in range(r)]
    side=[deque() for _ in range(r)]
    ops=iter(out.splitlines())
    turns=int(next(ops))
    assert 0<=turns<=4000
    operations=0
    transcript=[[] for _ in range(2*r)]
    for _ in range(turns):
        k=int(next(ops))
        assert 1<=k<=r
        pairs=[]
        for _ in range(k):
            t,i,j,q=map(int,next(ops).split())
            assert t in (0,1) and 0<=i<r and 0<=j<r and q>0
            pairs.append((i,j))
            row=(t,i,j,q)
            transcript[i].append(row)
            transcript[r+j].append(row)
            if t==0:
                assert len(dep[i])>=q and len(side[j])+q<=20
                moved=dep[i][-q:]
                del dep[i][-q:]
                side[j].extendleft(reversed(moved))
            else:
                assert len(side[j])>=q and len(dep[i])+q<=15
                dep[i].extend(side[j].popleft() for _ in range(q))
            operations+=1
        pairs.sort()
        assert all(a<c and b<d for (a,b),(c,d) in zip(pairs,pairs[1:]))
    assert next(ops,None) is None
    assert all(list(range(10*i,10*(i+1)))==dep[i] and not side[i] for i in range(r))
    return 100*r+4000-turns,turns,operations,transcript


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','vis','serial','cardinality','critical','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,serial,cardinality,critical=(getattr(a,k).resolve() for k in ('gen','vis','serial','cardinality','critical'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','turns','ops','base','cardinality','critical','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc064/dag-packing-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(30)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                records=[]
                for label,binary in [('serial',serial),('cardinality',cardinality),('critical',critical)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,turns,ops,transcript=replay(data,run.stdout)
                    detail=re.fullmatch(rb'base=(\d+) ops=(\d+) card=(\d+) critical=(\d+) turns=(\d+)\s*',run.stderr)
                    assert detail and (int(detail[2]),int(detail[5]))==(ops,turns) and elapsed<2,run.stderr
                    records.append((int(detail[1]),transcript))
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}',judge.stdout
                    w.writerow((split,seed,label,score,turns,ops,int(detail[1]),int(detail[3]),int(detail[4]),f'{elapsed:.6f}'))
                    file.flush()
                assert records[0]==records[1]==records[2]


if __name__=='__main__':
    main()
