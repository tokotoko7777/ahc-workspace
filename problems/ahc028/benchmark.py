"""Pair legal solutions on official-generator development/holdout seeds."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time

def replay(case,result):
    tokens=case.decode().split()
    n,m,r,c=map(int,tokens[:4])
    grid=tokens[4:4+n]
    words=tokens[4+n:]
    assert len(words)==m
    coords=[tuple(map(int,line.split())) for line in result.splitlines()]
    assert len(coords)<=5000
    output=[]
    cost=0
    for y,x in coords:
        assert 0<=y<n and 0<=x<n
        cost+=abs(y-r)+abs(x-c)+1
        r,c=y,x
        output.append(grid[r][c])
    s="".join(output)
    assert all(w in s for w in words)
    return max(10000-cost,1001),cost,len(coords)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","vis","greedy","tour","csv"):
        parser.add_argument(k,type=Path)
    args=parser.parse_args()
    gen,vis,greedy,tour=(getattr(args,k).resolve() for k in ("gen","vis","greedy","tour"))
    with tempfile.TemporaryDirectory() as temp,args.csv.open("w",newline="") as stream:
        tmp=Path(temp)
        seedfile,out=tmp/"seeds.txt",tmp/"answer.txt"
        writer=csv.writer(stream,lineterminator="\n")
        writer.writerow(("split","seed","arm","score","cost","keys","seconds"))
        for split in ("dev","holdout"):
            seeds=[int.from_bytes(hashlib.sha256(f"ahc028/ragged-tour-v1/{split}/{i}".encode()).digest()[:8],
                                  "little") for i in range(24)]
            seedfile.write_text("".join(f"{seed}\n" for seed in seeds))
            subprocess.run([gen,seedfile],cwd=tmp,check=True)
            for i,seed in enumerate(seeds):
                inp=tmp/"in"/f"{i:04}.txt"
                data=inp.read_bytes()
                arms=[("greedy",greedy),("tour",tour)]
                paired={}
                for label,binary in arms[::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    run=subprocess.run([binary],input=data,check=True,capture_output=True,timeout=2)
                    sec=time.monotonic()-start
                    score,cost,keycount=replay(data,run.stdout)
                    assert re.fullmatch(fr"cost={cost}\s*",run.stderr.decode()) and sec<2
                    out.write_bytes(run.stdout)
                    official=subprocess.run([vis,inp,out],cwd=tmp,capture_output=True,text=True,check=True)
                    assert official.stdout.strip()==f"Score = {score}",(split,seed,official.stdout)
                    paired[label]=score
                    writer.writerow((split,seed,label,score,cost,keycount,f"{sec:.6f}"))
                    stream.flush()
                assert paired["tour"]>=paired["greedy"]

if __name__=="__main__":
    main()
