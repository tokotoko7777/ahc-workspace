"""One frozen solve per fresh seed; score before and after relocating flips."""
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
    for k in ('gen','vis','solver','relocate','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,solver,relocate=(getattr(a,k).resolve() for k in ('gen','vis','solver','relocate'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        work=Path(d)
        seeds,answer=work/'seeds.txt',work/'answer.txt'
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','before','after','same_moves','ops','flips','deliveries','solver_seconds','relocate_seconds'))
        suffixes=[(s,i) for s,n in [('dev',6),('holdout',12)] for i in range(n)]
        ids=[int.from_bytes(hashlib.sha256(f'ahc060/fixed-walk-relocation-v1/{s}/{i}'.encode()).digest()[:8],
                            'little') for s,i in suffixes]
        seeds.write_text(''.join(f'{x}\n' for x in ids))
        subprocess.run([gen,seeds],cwd=work,check=True)
        for j,((split,_),seed) in enumerate(zip(suffixes,ids,strict=True)):
            inp=work/'in'/f'{j:04d}.txt'
            data=inp.read_bytes()
            start=time.monotonic()
            old=subprocess.run([solver],input=data,capture_output=True,timeout=3,check=True)
            solver_time=time.monotonic()-start
            start=time.monotonic()
            new=subprocess.run([relocate,inp],input=old.stdout,capture_output=True,timeout=1,check=True)
            relocate_time=time.monotonic()-start
            before,*meta=replay(data,old.stdout)
            after,*meta2=replay(data,new.stdout)
            def signature(output):
                at=0
                flips=set();moves=[]
                for v in map(int,output.split()):
                    if v<0: flips.add(at)
                    else: at=v;moves.append(v)
                return moves,flips
            assert after>=before and meta==meta2 and signature(old.stdout)==signature(new.stdout)
            assert re.search(rb'score=(\d+)',old.stderr).group(1)==str(before).encode()
            assert new.stderr.strip()==f'{before} -> {after}'.encode()
            for out,score in [(old.stdout,before),(new.stdout,after)]:
                answer.write_bytes(out)
                checked=subprocess.run([vis,inp,answer],cwd=work,capture_output=True,text=True,check=True)
                assert checked.stdout.strip()==f'Score = {score}',checked.stdout
            w.writerow((split,seed,before,after,True,*meta,f'{solver_time:.6f}',f'{relocate_time:.6f}'))
            file.flush()


if __name__=='__main__':
    main()
