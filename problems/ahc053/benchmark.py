"""Run official interaction and independently replay digit-card assignments."""
import argparse
import csv
import hashlib
import math
from pathlib import Path
import re
import subprocess
import tempfile
import time


def replay(data, output):
    n,m,l,u,*targets=map(int,data.split())
    assert len(targets)==m and all(l<=t<=u for t in targets)
    a,x=(list(map(int,row.split())) for row in output.splitlines())
    assert len(a)==len(x)==n
    assert all(1<=v<=u for v in a) and all(0<=p<=m for p in x)
    sums=[0]*(m+1)
    for v,p in zip(a,x,strict=True):
        sums[p]+=v
    error=sum(abs(t-v) for t,v in zip(targets,sums[1:],strict=True))
    score=math.floor((20-math.log10(1+error))*5e7+0.5)
    return score,error,tuple(a)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for key in ('gen','tester','base','match','csv'):
        parser.add_argument(key,type=Path)
    a=parser.parse_args()
    gen,tester,base,match=(getattr(a,k).resolve() for k in ('gen','tester','base','match'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','error','seconds'))
        work=Path(d)
        seeds=work/'seeds.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc053/rank-cards-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(32)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            build=None
            for i,s in enumerate(ids):
                data=(work/'in'/f'{i:04d}.txt').read_bytes()
                for label,binary in [('nearest',base),('match',match)][::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    run=subprocess.run([tester,binary],input=data,capture_output=True,check=True,timeout=2)
                    elapsed=time.monotonic()-start
                    score,error,deck=replay(data,run.stdout)
                    assert elapsed<2
                    assert re.search(fr'\bScore = {score}\s*$'.encode(),run.stderr),run.stderr
                    assert re.search(fr'\berror={error}\b'.encode(),run.stderr),run.stderr
                    if build is None:
                        build=deck
                    assert build==deck
                    w.writerow((split,s,label,score,error,f'{elapsed:.6f}'))
                    file.flush()


if __name__=='__main__':
    main()
