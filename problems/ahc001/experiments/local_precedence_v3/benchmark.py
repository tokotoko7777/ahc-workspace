"""Measure axis-preserving vs strict-axis neighborhood repairs from identical v64 outputs."""
import argparse
import csv
import hashlib
import importlib.util
import os
from pathlib import Path
import re
import subprocess
import time

module=Path(__file__).resolve().parents[2]/'benchmark.py'
spec=importlib.util.spec_from_file_location('rectangles',module)
checker=importlib.util.module_from_spec(spec)
spec.loader.exec_module(checker)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','vis','solver','repair','official_seeds','work'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,solver,repair,seeds,work=(getattr(a,k).resolve() for k in ('gen','vis','solver','repair','official_seeds','work'))
    work.mkdir(parents=True,exist_ok=True)
    banned=set(map(int,seeds.read_text().split()))
    for path in module.parent.glob('*seeds.txt'):
        banned.update(map(int,path.read_text().split()))
    for path in module.parent.glob('experiments/*/results.csv'):
        with path.open() as f:
            banned.update(int(row['seed']) for row in csv.DictReader(f) if 'seed' in row)
    with (work/'results.csv').open('w',newline='') as f:
        w=csv.writer(f,lineterminator='\n')
        w.writerow(('split','seed','arm','score','initial','attempts','feasible','improves','moved','gain','seconds'))
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc001/local-precedence-v3/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(6)]
            assert len(set(ids))==6 and not (set(ids)&banned)
            banned.update(ids)
            folder=work/split
            folder.mkdir(exist_ok=True)
            text=''.join(f'{s}\n' for s in ids)
            (folder/'seeds.txt').write_text(text)
            subprocess.run([gen],input=text,text=True,cwd=folder,check=True)
            for i,seed in enumerate(ids):
                inp=folder/'in'/f'{i:04}.txt'
                data=inp.read_bytes()
                start=time.monotonic()
                original=subprocess.run([solver,'4.8','0','64'],input=data,capture_output=True,timeout=5,check=True)
                baseline_seconds=time.monotonic()-start
                initial=checker.check(data,original.stdout)
                path=folder/f'{i:04}-same-start.txt'
                path.write_bytes(original.stdout)
                arms=[('retaining',False),('opposite',True)]
                records=[('none',original.stdout,(0,0,0,0,0),baseline_seconds)]
                for name,twist in arms[::(-1 if i%2 else 1)]:
                    env=dict(os.environ,AHC001_LOCAL_TWIST=str(int(twist)))
                    start=time.monotonic()
                    result=subprocess.run([repair,inp,path],capture_output=True,check=True,timeout=1,env=env)
                    elapsed=time.monotonic()-start
                    m=re.fullmatch(rb'attempts=(\d+) feasible=(\d+) improves=(\d+) moved=(\d+) gain=(\d+)\s*',result.stderr)
                    assert m,result.stderr
                    records.append((name,result.stdout,tuple(map(int,m.groups())),elapsed))
                for name,output,diag,elapsed in records:
                    score=checker.check(data,output)
                    assert score>=initial and abs(score-initial-diag[-1])<=1
                    ans=folder/f'{i:04}-{name}.txt'
                    ans.write_bytes(output)
                    official=subprocess.run([vis,inp,ans],cwd=folder,capture_output=True,text=True,check=True)
                    assert official.stdout.strip()==str(score) and not official.stderr
                    w.writerow((split,seed,name,score,initial,*diag,f'{elapsed:.6f}'))
                    f.flush()
                assert records[1][2][0]==records[2][2][0]
                print(split,i,flush=True)


if __name__=='__main__':
    main()
