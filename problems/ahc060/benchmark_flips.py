"""Compare delivery-path search using official vis and independent graph replay."""
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
    for k in ('gen','vis','frozen','wider','flips','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,frozen,wider,flips=(getattr(a,k).resolve() for k in ('gen','vis','frozen','wider','flips'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','ops','red','deliveries','initial','probes','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc060/flip-macro-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(24)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                shared=[]
                for label,binary in [('frozen',frozen),('wider',wider),('flips',flips)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,ops,red,delivered=replay(data,run.stdout)
                    detail=re.fullmatch(rb'base=(\d+) score=(\d+) probes=(\d+)\s*',run.stderr)
                    assert detail and int(detail[2])==score and elapsed<2,run.stderr
                    shared.append(int(detail[1]))
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}',judge.stdout
                    w.writerow((split,seed,label,score,ops,red,delivered,shared[-1],int(detail[3]),f'{elapsed:.6f}'))
                    file.flush()
                assert len(set(shared))==1


if __name__=='__main__':
    main()
