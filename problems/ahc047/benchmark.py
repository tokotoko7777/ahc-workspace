"""Compare exact official Markov scores to independent matrix/substring replay."""
import argparse
import csv
import hashlib
import math
from pathlib import Path
import re
import subprocess
import tempfile
import time
import numpy as np

def verify(inp,out):
    data=inp.decode().split()
    n,m,L=map(int,data[:3])
    words=list(zip(data[3::2],map(int,data[4::2]),strict=True))
    assert len(words)==n
    lines=[line.split() for line in out.decode().splitlines()]
    assert len(lines)==m
    labels=[x[0] for x in lines]
    assert all(len(c)==1 and "a"<=c<="f" for c in labels)
    probabilities=[list(map(int,x[1:])) for x in lines]
    assert all(len(row)==m and min(row)>=0 and max(row)<=100 and sum(row)==100 for row in probabilities)
    objective=0.
    for word,weight in words:
        prefix=[""]+[word[:q] for q in range(1,len(word))]
        pairs=[(j,q) for j,c in enumerate(labels) for q,p in enumerate(prefix) if not p or p[-1]==c]
        index={p:i for i,p in enumerate(pairs)}
        a=np.zeros((len(pairs),len(pairs)),dtype=np.float64)
        for i,(s,q) in enumerate(pairs):
            for j,c in enumerate(labels):
                if not probabilities[s][j]:continue
                one=prefix[q]+c
                if one.endswith(word):continue
                new=max(k for k,p in enumerate(prefix) if one.endswith(p))
                a[i,index[j,new]]+=probabilities[s][j]/100
        start=index[0,int(labels[0]==word[0])]
        v=np.linalg.matrix_power(a,L-1)[start].sum()
        objective+=weight*min(1.,max(0.,1-v))
    return math.floor(objective+0.5),objective

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ("gen","vis","base","candidate","csv"):
        p.add_argument(name,type=Path)
    a=p.parse_args()
    gen,vis,base,candidate=(getattr(a,k).resolve() for k in ("gen","vis","base","candidate"))
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as file:
        work=Path(d)
        seed,answer=work/"seeds.txt",work/"answer.txt"
        w=csv.writer(file,lineterminator="\n")
        w.writerow(("split","seed","arm","score","expectation","baseline","seconds"))
        for split in ("dev","holdout"):
            ids=[int.from_bytes(hashlib.sha256(f"ahc047/markov-mixture-v1/{split}/{i}".encode()).digest()[:8],
                                "little") for i in range(24)]
            seed.write_text("".join(f"{s}\n" for s in ids))
            subprocess.run([gen,seed],cwd=work,check=True)
            for i,s in enumerate(ids):
                inp=work/"in"/f"{i:04d}.txt"
                data=inp.read_bytes()
                arms=[("cycle",base),("mixture",candidate)]
                starts=[]
                for label,binary in arms[::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    run=subprocess.run([binary],input=data,check=True,capture_output=True,timeout=2)
                    elapsed=time.monotonic()-start
                    score,exact=verify(data,run.stdout)
                    found=re.fullmatch(rb"base=(\d+) score=(\d+)\s*",run.stderr)
                    assert found and int(found[2])==score and elapsed<2
                    starts.append(int(found[1]))
                    answer.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,answer],cwd=work,check=True,capture_output=True,text=True)
                    assert judge.stdout.strip()==f"Score = {score}",judge.stdout
                    w.writerow((split,s,label,score,f"{exact:.8f}",starts[-1],f"{elapsed:.6f}"))
                assert len(set(starts))==1
                file.flush()

if __name__=="__main__":
    main()
