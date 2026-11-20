"""Paired exact official score checks using independent alternating-edge replay."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time

def validate(data,out):
    n,L,*target=map(int,data.split())
    edge=[tuple(map(int,line.split())) for line in out.splitlines()]
    assert len(target)==n and len(edge)==n
    assert all(len(r)==2 and 0<=min(r)<=max(r)<n for r in edge)
    count=[0]*n
    forward=[0]*n
    employee=0
    for _ in range(L):
        count[employee]+=1
        old=employee
        employee=edge[employee][forward[employee]]
        forward[old]^=1
    return sum(abs(a-b) for a,b in zip(count,target,strict=True))

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","vis","base","candidate","csv"):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,base,candidate=(getattr(a,k).resolve() for k in ("gen","vis","base","candidate"))
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as f:
        work=Path(d)
        seeds,answer=work/"seeds.txt",work/"answer.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","arm","score","error","start","seconds"))
        for split in ("dev","holdout"):
            ids=[int.from_bytes(hashlib.sha256(f"ahc044/rotor-residual-v1/{split}/{i}".encode()).digest()[:8],
                                "little") for i in range(24)]
            seeds.write_text("".join(f"{s}\n" for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/"in"/f"{i:04d}.txt"
                data=inp.read_bytes()
                arms=[("greedy",base),("anneal",candidate)]
                starts=[]
                for label,binary in arms[::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    run=subprocess.run([binary],input=data,check=True,capture_output=True,timeout=2)
                    seconds=time.monotonic()-start
                    error=validate(data,run.stdout)
                    detail=re.fullmatch(rb"start=(\d+) error=(\d+)\s*",run.stderr)
                    assert detail and error==int(detail[2]) and seconds<2
                    score=10**6-error
                    starts.append(int(detail[1]))
                    answer.write_bytes(run.stdout)
                    official=subprocess.run([vis,inp,answer],cwd=work,capture_output=True,text=True,check=True)
                    assert official.stdout.strip()==f"Score = {score}",official
                    w.writerow((split,seed,label,score,error,starts[-1],f"{seconds:.6f}"))
                    f.flush()
                assert starts[0]==starts[1]

if __name__=="__main__":
    main()
