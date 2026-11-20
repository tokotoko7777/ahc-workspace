"""Exact official and independent paired AHC072 packet benchmark."""
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
    names=('gen','vis','baseline','candidate','official','work')
    for name in names:p.add_argument(name,type=Path)
    a=p.parse_args()
    gen,vis,baseline,candidate,official,work=(getattr(a,k).resolve() for k in names)
    banned=set(map(int,official.read_text().split()))
    for f in problem.glob('*results.csv'):
        with f.open() as src:banned.update(int(x['seed']) for x in csv.DictReader(src))
    work.mkdir(parents=True,exist_ok=True)
    with (work/'results.csv').open('w',newline='') as out:
        w=csv.writer(out,lineterminator='\n')
        w.writerow(('split','seed','arm','moves','slimes','long_jumps','max_height',
                    'single_base','frozen','mixed','packet_trials','reachable','accepted','saved','seconds'))
        for split,count in (('dev',12),('holdout',20)):
            folder=work/split;folder.mkdir(exist_ok=True)
            ids=[int.from_bytes(hashlib.sha256(f'ahc072/fixed-packet-v3/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(count)]
            assert len(set(ids))==count and not (set(ids)&banned)
            banned.update(ids)
            seeds=folder/'seeds.txt';seeds.write_text(''.join(f'{x}\n' for x in ids))
            subprocess.run([gen,seeds],cwd=folder,check=True)
            for i,seed in enumerate(ids):
                inp=folder/'in'/f'{i:04}.txt';data=inp.read_bytes()
                start=set()
                for arm,binary in [('caravan',baseline),('packet',candidate)][::(-1 if i%2 else 1)]:
                    before=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,check=True,timeout=2)
                    elapsed=time.monotonic()-before
                    ops,*stats=replay(data,run.stdout)
                    m=re.fullmatch(rb'base=(\d+) moves=(\d+)(?: frozen=(\d+) mixed=(\d+) packet_trials=(\d+) reachable=(\d+) accepted=(\d+) saved=(\d+))?\s*',run.stderr)
                    assert m and int(m[2])==ops and elapsed<2,run.stderr
                    base=int(m[1]);extra=(ops,0,0,0,0,0) if arm=='caravan' else tuple(map(int,m.groups()[2:]))
                    assert ops<=extra[0] and (extra[1]==0 or ops==min(extra[:2]))
                    start.add((base,extra[0]))
                    ans=folder/f'{i:04}-{arm}.txt';ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=folder,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {ops}',judge.stdout
                    w.writerow((split,seed,arm,ops,*stats,base,*extra,f'{elapsed:.6f}'))
                    out.flush()
                assert len(start)==1
                print(split,i,flush=True)

if __name__=='__main__':main()
