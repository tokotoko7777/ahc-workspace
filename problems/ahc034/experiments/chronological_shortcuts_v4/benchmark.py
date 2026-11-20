"""Official and independent replay for chronological AHC034 walk exchanges."""
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
from benchmark import validate

def main():
    p=argparse.ArgumentParser(description=__doc__)
    names=('gen','vis','baseline','control','coverage','candidate','starter_seeds','work')
    for k in names:p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,b,c,coverage,s,starter,work=[getattr(a,k).resolve() for k in names]
    work.mkdir(parents=True,exist_ok=True)
    banned=set(map(int,starter.read_text().split()))
    for f in problem.glob('*results.csv'):
        with f.open() as src:banned.update(int(x['seed']) for x in csv.DictReader(src))
    with (problem/'experiments/physical_shortcuts_v3/results.csv').open() as f:
        banned.update(int(x['seed']) for x in csv.DictReader(f))
    for i in range(20):
        banned.add(int.from_bytes(hashlib.sha256(f'ahc034/lower-bound-diagnostic-v1/{i}'.encode()).digest()[:8],'little'))
    with (work/'results.csv').open('w',newline='') as out:
        w=csv.writer(out,lineterminator='\n')
        w.writerow(('split','seed','arm','score','travel','handling','commands','steps','carried',
                    'inherited','original_steps','min_steps','trials','attempts','edits','accepted','gates','rejected','seconds'))
        for split,count in (('dev',10),('holdout',16)):
            path=work/split;path.mkdir(exist_ok=True)
            seeds=[int.from_bytes(hashlib.sha256(f'ahc034/chronological-shortcuts-v4/{split}/{i}'.encode()).digest()[:8],
                                  'little') for i in range(count)]
            assert not (set(seeds)&banned) and len(set(seeds))==count
            banned.update(seeds)
            seed_file=path/'seeds.txt';seed_file.write_text(''.join(f'{x}\n' for x in seeds))
            subprocess.run([gen,seed_file],cwd=path,check=True)
            for i,seed in enumerate(seeds):
                inp=path/'in'/f'{i:04}.txt';data=inp.read_bytes()
                starts=set()
                for label,binary in [('baseline',b),('long',c),('coverage',coverage),('chronological',s)][::(-1 if i%2 else 1)]:
                    before=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,check=True,timeout=2)
                    elapsed=time.monotonic()-before
                    score,travel,h,commands=validate(data,run.stdout)
                    steps=sum(x in (b'U',b'D',b'L',b'R') for x in run.stdout.splitlines())
                    msg=re.fullmatch(rb'start=(\d+) move=(\d+) fixed=(\d+) walk=(\d+)(?: inherited=(\d+) original_steps=(\d+) min_steps=(\d+) trials=(\d+) attempts=(\d+) edits=(\d+) better=(\d+)(?: gates=(\d+) rejected=(\d+))?)?\s*',run.stderr)
                    assert msg and (int(msg[2]),int(msg[4]))==(travel,steps),run.stderr
                    diag=(travel,steps,steps,0,0,0,0,0,0) if label=='baseline' else tuple(int(x or 0) for x in msg.groups()[4:])
                    assert travel<=diag[0] and diag[3]==(0 if label=='baseline' else 2)
                    assert (diag[7]>0)==(label=='chronological') and 0<=diag[8]<=diag[7]
                    starts.add((int(msg[1]),int(msg[3]),*diag[:2]))
                    output=path/f'{i:04}-{label}.txt';output.write_bytes(run.stdout)
                    official=subprocess.run([vis,inp,output],cwd=path,capture_output=True,text=True,check=True)
                    assert elapsed<2 and official.stdout.strip()==f'Score = {score}',official
                    w.writerow((split,seed,label,score,travel,h,commands,steps,travel-100*steps,*diag,f'{elapsed:.6f}'))
                    out.flush()
                assert len(starts)==1
                print(split,i,flush=True)

if __name__=='__main__':
    main()
