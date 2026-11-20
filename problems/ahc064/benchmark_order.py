"""Official visualizer and independent track/capacity/commutation replay."""
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
    for k in ('gen','vis','frozen','random','guided','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,frozen,random,guided=(getattr(a,k).resolve() for k in ('gen','vis','frozen','random','guided'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w=csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','turns','ops','frozen','trials','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc064/extraction-order-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(30)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                records=[]
                for label,binary in [('frozen',frozen),('random',random),('guided',guided)][::(-1 if i%2 else 1)]:
                    begin=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,timeout=2,check=True)
                    elapsed=time.monotonic()-begin
                    score,turns,ops,transcript=replay(data,run.stdout)
                    if label=='frozen':
                        detail=re.fullmatch(rb'base=(\d+) ops=(\d+) card=(\d+) critical=(\d+) turns=(\d+)\s*',run.stderr)
                        assert detail and (int(detail[2]),int(detail[5]))==(ops,turns)
                        baseline,trials=turns,0
                    else:
                        detail=re.fullmatch(rb'frozen=(\d+) ops=(\d+) trials=(\d+) turns=(\d+)\s*',run.stderr)
                        assert detail and (int(detail[2]),int(detail[4]))==(ops,turns)
                        baseline,trials=int(detail[1]),int(detail[3])
                        assert trials==32 and turns<=baseline
                    assert elapsed<2
                    records.append(baseline)
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}',judge.stdout
                    w.writerow((split,seed,label,score,turns,ops,baseline,trials,f'{elapsed:.6f}'))
                    file.flush()
                assert records[0]==records[1]==records[2]


if __name__=='__main__':
    main()
