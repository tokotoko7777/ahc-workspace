"""Run fresh paired tests and validate geometry and exact rational score."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time
from benchmark import replay


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','vis','frozen','restart','graft','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,frozen,restart,graft=(getattr(a,k).resolve() for k in ('gen','vis','frozen','restart','graft'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w = csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','leaves','base','frozen','n','m','k','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc051/reconvergence-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(28)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,s in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                starts=[]
                for label,binary in [('frozen',frozen),('restart',restart),('graft',graft)][::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,check=True,timeout=2)
                    elapsed=time.monotonic()-start
                    score,leaves=replay(data.splitlines(),run.stdout)
                    detail=re.fullmatch(rb'base=(\d+) final=(\d+)(?: frozen=(\d+))?\s*',run.stderr)
                    assert detail and int(detail[2])==score and elapsed<2
                    starts.append(int(detail[3] or detail[2]))
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}',judge.stdout
                    w.writerow((split,s,label,score,leaves,int(detail[1]),starts[-1],*map(int,data.splitlines()[0].split()),f'{elapsed:.6f}'))
                    file.flush()
                assert len(set(starts))==1


if __name__=='__main__':
    main()
