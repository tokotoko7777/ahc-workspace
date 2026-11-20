"""Validate frozen AHC066, fixed order and joint task-order/macro candidates."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import sys
import time

problem=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(problem))
from benchmark import replay

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','vis','baseline','fixed','joint','official','work'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,b,l,j,official,work=(getattr(a,k).resolve() for k in
                                   ('gen','vis','baseline','fixed','joint','official','work'))
    work.mkdir(parents=True,exist_ok=True)
    banned=set(map(int,official.read_text().split()))
    for f in problem.glob('benchmark*results.csv'):
        with f.open() as src:banned.update(int(x['seed']) for x in csv.DictReader(src))
    with (work/'results.csv').open('w',newline='') as dst:
        w=csv.writer(dst,lineterminator='\n')
        w.writerow(('split','seed','arm','n','m','T','score','expanded','plays','before_joint','matrices','probes','seconds'))
        for split,count in (('dev',16),('holdout',24)):
            path=work/split;path.mkdir(exist_ok=True)
            ids=[int.from_bytes(hashlib.sha256(f'ahc066/joint-order-v2/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(count)]
            assert len(set(ids))==count and not (banned&set(ids))
            banned.update(ids)
            seeds=path/'seeds.txt'
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=path,check=True)
            for i,seed in enumerate(ids):
                inp=path/'in'/f'{i:04}.txt'
                data=inp.read_bytes()
                n,m,T=map(int,data.splitlines()[0].split())
                starts=set()
                for label,binary in [('baseline',b),('fixed',l),('joint',j)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,expanded,balls,plays=replay(data,run.stdout)
                    assert balls==m and score<=T and expanded<=T and elapsed<2
                    detail=re.fullmatch(rb'literal=\d+ base=\d+ score=(\d+) expanded=(\d+) frozen=\d+ trials=10(?: before_joint=(\d+) matrices=(\d+) probes=(\d+))?\s*',run.stderr)
                    assert detail and (int(detail[1]),int(detail[2]))==(score,expanded),run.stderr
                    fallback,matrices,probes=(score,0,0) if label=='baseline' else map(int,detail.groups()[2:])
                    assert score<=fallback and matrices==(0 if label=='baseline' else 6)
                    assert probes==(48*6 if label=='joint' else 0)
                    starts.add(fallback)
                    ans=path/f'{i:04}-{label}.txt'
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=path,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}' and not judge.stderr,judge
                    w.writerow((split,seed,label,n,m,T,score,expanded,plays,fallback,matrices,probes,f'{elapsed:.6f}'))
                    dst.flush()
                assert len(starts)==1
                print(split,i,flush=True)

if __name__=='__main__':
    main()
