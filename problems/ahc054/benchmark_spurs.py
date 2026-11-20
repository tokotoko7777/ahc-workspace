"""Judge an interactive forest plan with the official tester and independent replay."""
import argparse
from collections import deque
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
    for k in ('gen','tester','frozen','restart','spurs','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,tester,frozen,restart,spurs=(getattr(a,k).resolve()
                                      for k in ('gen','tester','frozen','restart','spurs'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','planted','path','n','seconds'))
        work=Path(d)
        seeds=work/'seeds.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc054/spurs-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(24)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                data=(work/'in'/f'{i:04d}.txt').read_bytes()
                for label,binary in [('frozen',frozen),('restart',restart),('spurs',spurs)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([tester,binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,planted,path=replay(data,run.stdout)
                    m=re.fullmatch(rb'path=(\d+)\s+Score = (\d+)\s*',run.stderr)
                    assert m and int(m[2])==score and elapsed<2,run.stderr
                    assert score>=path and int(m[1])==path
                    w.writerow((split,seed,label,score,planted,path,int(data.split()[0]),f'{elapsed:.6f}'))
                    file.flush()


if __name__=='__main__':
    main()
