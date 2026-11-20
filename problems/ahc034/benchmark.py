"""Check actual truck movement, load and cost against the official scorer."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time

def validate(inp,out):
    n,*h=list(map(int,inp.split()))
    assert len(h)==n*n and not sum(h)
    base=sum(abs(x) for x in h)
    r=c=load=travel=handling=0
    moves={b"U":(-1,0),b"D":(1,0),b"L":(0,-1),b"R":(0,1)}
    lines=out.splitlines()
    assert len(lines)<=100000
    for act in lines:
        if act in moves:
            dr,dc=moves[act]
            r+=dr;c+=dc
            assert 0<=r<n and 0<=c<n
            travel+=100+load
        else:
            assert re.fullmatch(rb"[+-][1-9][0-9]*",act)
            q=int(act)
            assert 0<abs(q)<=10**6
            h[r*n+c]-=q;load+=q;handling+=abs(q)
            assert load>=0
    assert not any(h) and load==0 and handling==base
    cost=travel+handling
    return (2*10**9*base+cost)//(2*cost),travel,handling,len(lines)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","vis","base","candidate","csv"):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,base,candidate=(getattr(a,k).resolve() for k in ("gen","vis","base","candidate"))
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as stream:
        working=Path(d)
        seeds,output=working/"seeds.txt",working/"answer.txt"
        writer=csv.writer(stream,lineterminator="\n")
        writer.writerow(("split","seed","arm","score","travel_cost","handling_cost","commands","initial_travel","seconds"))
        for split in ("dev","holdout"):
            ids=[int.from_bytes(hashlib.sha256(f"ahc034/prefix-load-v1/{split}/{i}".encode()).digest()[:8],
                               "little") for i in range(24)]
            seeds.write_text("".join(f"{seed}\n" for seed in ids))
            subprocess.run([gen,seeds],cwd=working,check=True)
            for i,seed in enumerate(ids):
                case=working/"in"/f"{i:04d}.txt"
                data=case.read_bytes()
                arms=[("snake",base),("anneal",candidate)]
                paired={}
                for label,binary in arms[::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    result=subprocess.run([binary],input=data,capture_output=True,check=True,timeout=2)
                    elapsed=time.monotonic()-start
                    outcomes=validate(data,result.stdout)
                    m=re.fullmatch(rb"start=(\d+) move=(\d+)\s*",result.stderr)
                    assert m and int(m[2])==outcomes[1]
                    output.write_bytes(result.stdout)
                    official=subprocess.run([vis,case,output],cwd=working,capture_output=True,check=True,text=True)
                    assert elapsed<2 and official.stdout.strip()==f"Score = {outcomes[0]}",official.stdout
                    paired[label]=int(m[1])
                    writer.writerow((split,seed,label,*outcomes,paired[label],f"{elapsed:.6f}"))
                    stream.flush()
                assert paired["snake"]==paired["anneal"]

if __name__=="__main__":
    main()
