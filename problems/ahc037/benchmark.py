"""Score monotone arborescences with official scorer and independent replay."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time

def validate(case,out):
    n,*coords=map(int,case.split())
    targets=set(zip(coords[::2],coords[1::2]))
    assert len(targets)==n
    m,*steps=map(int,out.split())
    assert 0<=m<=5*n and len(steps)==4*m
    present={(0,0)}
    total=0
    for x,y,a,b in zip(steps[::4],steps[1::4],steps[2::4],steps[3::4],strict=True):
        assert 0<=x<=a<10**9 and 0<=y<=b<10**9 and (x,y) in present
        present.add((a,b))
        total+=a+b-x-y
    assert targets<=present
    dividend=10**6*n*max(coords)
    return (2*dividend+1+total)//(2*(1+total)),total,m,len(present-targets-{(0,0)})

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","vis","control","candidate","csv"):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,control,candidate=(getattr(a,k).resolve() for k in ("gen","vis","control","candidate"))
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as f:
        work=Path(d)
        sf,answer=work/"seeds.txt",work/"answer.txt"
        writer=csv.writer(f,lineterminator="\n")
        writer.writerow(("split","seed","arm","score","cost","operations","junctions","seconds"))
        for split in ("dev","holdout"):
            seeds=[int.from_bytes(hashlib.sha256(f"ahc037/bottom-left-joins-v1/{split}/{i}".encode()).digest()[:8],
                                "little") for i in range(28)]
            sf.write_text("".join(f"{x}\n" for x in seeds))
            subprocess.run([gen,sf],cwd=work,check=True)
            for i,seed in enumerate(seeds):
                inp=work/"in"/f"{i:04d}.txt"
                data=inp.read_bytes()
                arms=[("terminal",control),("join",candidate)]
                paired={}
                for label,binary in arms[::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,check=True,timeout=2)
                    elapsed=time.monotonic()-start
                    score,cost,count,junctions=validate(data,run.stdout)
                    m=re.fullmatch(rb"base=(\d+) cost=(\d+)\s*",run.stderr)
                    assert m and int(m[2])==cost and elapsed<2
                    answer.write_bytes(run.stdout)
                    official=subprocess.run([vis,inp,answer],cwd=work,capture_output=True,text=True,check=True)
                    assert official.stdout.strip()==f"Score = {score}",(score,official.stdout)
                    paired[label]=int(m[1]),cost
                    writer.writerow((split,seed,label,score,cost,count,junctions,f"{elapsed:.6f}"))
                    f.flush()
                assert paired["terminal"][0]==paired["join"][0]
                assert paired["terminal"][1]>=paired["join"][1]

if __name__=="__main__":
    main()
