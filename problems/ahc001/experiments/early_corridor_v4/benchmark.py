"""Preregistered fresh 4.8 s paired early-corridor comparison."""
import argparse
import csv
import hashlib
import importlib.util
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
    for k in ('gen','vis','base','corridor','official_seeds','work'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,base,corridor,official,work=(getattr(a,k).resolve() for k in ('gen','vis','base','corridor','official_seeds','work'))
    work.mkdir(parents=True,exist_ok=True)
    banned=set(map(int,official.read_text().split()))
    problem=module.parent
    for path in problem.glob('*seeds.txt'):
        banned.update(map(int,path.read_text().split()))
    all_seeds=set()
    inputs={}
    banned.update(int.from_bytes(hashlib.sha256(f'ahc001-topology-v1-dev-{i}'.encode()).digest()[:8],
                                 'big') for i in range(8))
    for version in ('forced_precedence_v2','local_precedence_v3'):
        with (problem/'experiments'/version/'results.csv').open() as f:
            banned.update(int(row['seed']) for row in csv.DictReader(f))
    for split, count in (('dev',8),('holdout',12)):
        ids=[int.from_bytes(hashlib.sha256(f'ahc001/early-corridor-v4/{split}/{i}'.encode()).digest()[:8],
                            'little') for i in range(count)]
        assert len(set(ids))==count and not (set(ids)&(banned|all_seeds))
        all_seeds.update(ids)
        path=work/split
        path.mkdir(exist_ok=True)
        text=''.join(f'{s}\n' for s in ids)
        (path/'seeds.txt').write_text(text)
        subprocess.run([gen],input=text,text=True,cwd=path,check=True)
        inputs[split]=[(seed,path/'in'/f'{i:04d}.txt') for i,seed in enumerate(ids)]
    with (work/'results.csv').open('w',newline='') as f:
        w=csv.writer(f,lineterminator='\n')
        w.writerow(('split','seed','arm','n','score','iterations','regional_trials','regional_accepted',
                    'pairs_considered','biaxis','pairs_selected','admitted','exclusive_survivors','initial_gain','seconds'))
        for split, selected in inputs.items():
            for i,(seed,inp) in enumerate(selected):
                data=inp.read_bytes()
                n=int(data.split()[0])
                for label,binary in [('v64',base),('corridor',corridor)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary,'4.8','0','64'],input=data,capture_output=True,timeout=5,check=True)
                    elapsed=time.monotonic()-begin
                    assert elapsed<5
                    score=checker.check(data,run.stdout)
                    logs=run.stderr.decode()
                    common=re.search(r'score=(\d+) iterations=(\d+).*\nregional_trials=(\d+) regional_improvements=(\d+)',logs)
                    assert common and int(common[1])==score,logs
                    detail=re.search(r'early_candidates=(\d+) biaxis=(\d+) pairs=(\d+) admitted=(\d+) exclusive_survivors=(\d+) initial_gain=(-?\d+)',logs)
                    assert (label=='corridor')==bool(detail),logs
                    answer=work/f'{split}-{i:02d}-{label}.txt'
                    answer.write_bytes(run.stdout)
                    official_score=subprocess.run([vis,inp,answer],cwd=work,capture_output=True,text=True,check=True)
                    assert official_score.stdout.strip()==str(score) and not official_score.stderr,official_score
                    (work/f'{split}-{i:02d}-{label}.log').write_text(logs)
                    w.writerow((split,seed,label,n,score,*map(int,common.groups()[1:]),
                                *(map(int,detail.groups()) if detail else (0,0,0,0,0,0)),f'{elapsed:.6f}'))
                    f.flush()
                    print(split,label,i,score,flush=True)


if __name__=='__main__':
    main()
