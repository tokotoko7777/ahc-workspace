"""Exact official scoring and independent stack damage replay."""
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
    for k in ("gen","vis","base","candidate","csv"):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,base,cand=(getattr(a,k).resolve() for k in ("gen","vis","base","candidate"))
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as file:
        w=csv.writer(file,lineterminator="\n")
        w.writerow(("split","seed","arm","score","moves","trips","capacity","durability_min","commands","base","probes","seconds"))
        work=Path(d); seeds,ans=work/"seeds.txt",work/"answer.txt"
        for split in ("dev","holdout"):
            ids=[int.from_bytes(hashlib.sha256(f"ahc049/anchor-support-v1/{split}/{i}".encode()).digest()[:8],
                                "little") for i in range(28)]
            seeds.write_text("".join(f"{s}\n" for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,s in enumerate(ids):
                inp=work/"in"/f"{i:04d}.txt"
                data=inp.read_bytes()
                arms=[("fixed",base),("diverse",cand)]
                starts=[]
                for label,binary in arms[::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,check=True,timeout=2)
                    elapsed=time.monotonic()-start
                    score=replay(data,run.stdout)
                    detail=re.fullmatch(rb"base=(\d+) moves=(\d+) probes=(\d+)\s*",run.stderr)
                    assert detail and int(detail[2])==score[1] and elapsed<2
                    starts.append(int(detail[1]))
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f"Score = {score[0]}",judge.stdout
                    w.writerow((split,s,label,*score,starts[-1],int(detail[3]),f"{elapsed:.6f}"))
                    file.flush()
                assert starts[0]==starts[1]

if __name__=="__main__":
    main()
