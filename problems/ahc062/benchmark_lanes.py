"""Official and independent weighted king-path evaluation."""
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
    for k in ('gen','vis','twoopt','lanes','csv'):
        p.add_argument(k,type=Path)
    p.add_argument("--count",type=int,default=18)
    a=p.parse_args()
    gen,vis,twoopt,lanes=(getattr(a,k).resolve() for k in ('gen','vis','twoopt','lanes'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','raw','initial','trials','accepted','endpoint','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc062/lane-split-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(a.count)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                initial=[]
                for label,binary in [('twoopt',twoopt),('lanes',lanes)][::(-1 if i%2 else 1)]:
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
                assert all(x>0 for x in initial)


if __name__=='__main__':
    main()
