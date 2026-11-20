"""Run official interaction and independently replay digit-card assignments."""
import argparse
import csv
import hashlib
import math
from pathlib import Path
import re
import subprocess
import tempfile
import time


from benchmark import replay


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for key in ('gen','tester','base','match','pair','csv'):
        parser.add_argument(key,type=Path)
    a=parser.parse_args()
    gen,tester,base,match,pair=(getattr(a,k).resolve() for k in ('gen','tester','base','match','pair'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','error','seconds'))
        work=Path(d)
        seeds=work/'seeds.txt'
        for split in ('dev-a','dev-b','holdout-a','holdout-b','holdout-c'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc053/paired-pools-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(20)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            decks={}
            for i,s in enumerate(ids):
                data=(work/'in'/f'{i:04d}.txt').read_bytes()
                for label,binary in [('nearest',base),('match',match),('pair',pair)][::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    run=subprocess.run([tester,binary],input=data,capture_output=True,check=True,timeout=2)
                    elapsed=time.monotonic()-start
                    score,error,deck=replay(data,run.stdout)
                    assert elapsed<2
                    assert re.search(fr'\bScore = {score}\s*$'.encode(),run.stderr),run.stderr
                    assert re.search(fr'\berror={error}\b'.encode(),run.stderr),run.stderr
                    family=label if label=="pair" else "legacy"
                    if family not in decks: decks[family]=deck
                    assert decks[family]==deck
                    w.writerow((split,s,label,score,error,f'{elapsed:.6f}'))
                    file.flush()


if __name__=='__main__':
    main()
