"""Replay the grid machine and compare to official visualizer."""
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
    for k in ('gen','vis','frozen','random','compress','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,frozen,random,compress=(getattr(a,k).resolve() for k in ('gen','vis','frozen','random','compress'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','colors','states','rules','steps','frozen','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc056/route-ties-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(32)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                records=[]
                for label,binary in [('frozen',frozen),('random',random),('compress',compress)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,c,q,m,step=replay(data,run.stdout)
                    detail=re.fullmatch(rb'base=(\d+) cost=(\d+) steps=(\d+)(?: frozen=(\d+))?\s*',run.stderr)
                    assert detail and (int(detail[2]),int(detail[3]))==(score,step) and elapsed<2,run.stderr
                    records.append((int(detail[4] or detail[2]),step))
                    ans.write_bytes(run.stdout)
                    official=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert official.stdout.strip()==f'Score = {score}',official.stdout
                    w.writerow((split,seed,label,score,c,q,m,step,records[-1][0],f'{elapsed:.6f}'))
                    file.flush()
                assert len(set(records))==1


if __name__=='__main__':
    main()
