"""Full independent somersault simulation and official score comparison."""
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
    for k in ('gen','vis','frozen','random','batch','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,frozen,random,batch=(getattr(a,k).resolve() for k in ('gen','vis','frozen','random','batch'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','slimes','long_jumps','max_height','frozen','trials','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc072/global-batches-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(30)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                starts=[]
                for label,binary in [('frozen',frozen),('random',random),('batch',batch)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,done,jumps,height=replay(data,run.stdout)
                    detail=re.fullmatch(rb'base=(\d+) moves=(\d+)(?: frozen=(\d+) trials=(\d+))?\s*',run.stderr)
                    assert detail and int(detail[2])==score and elapsed<2,run.stderr
                    starts.append(int(detail[3] or score))
                    assert int(detail[4] or 0)==(0 if label=='frozen' else 8)
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}',judge.stdout
                    w.writerow((split,seed,label,score,done,jumps,height,starts[-1],int(detail[4] or 0),f'{elapsed:.6f}'))
                    file.flush()
                assert len(set(starts))==1


if __name__=='__main__':
    main()
