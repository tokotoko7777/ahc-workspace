"""Paired exact skate commands using independent stepwise movement replay."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time

def replay(data,out):
    n,m,*coords=map(int,data.split())
    goals=list(zip(coords[::2],coords[1::2],strict=True))
    assert len(goals)==m
    pos=goals[0]
    nxt=1
    walls=set()
    edits=slides=0
    d={b"U":(-1,0),b"D":(1,0),b"L":(0,-1),b"R":(0,1)}
    lines=out.splitlines()
    assert len(lines)<=2*n*m
    def inside(p):return 0<=p[0]<n and 0<=p[1]<n
    for line in lines:
        op,direction=line.split()
        a,b=d[direction]
        def move(p):return p[0]+a,p[1]+b
        q=move(pos)
        if op==b"A":
            assert inside(q)
            walls.symmetric_difference_update({q})
            edits+=1
            continue
        if op==b"M":
            assert inside(q) and q not in walls
            pos=q
        elif op==b"S":
            slides+=1
            while inside(q) and q not in walls:
                pos=q;q=move(pos)
        else:raise AssertionError(op)
        if nxt<m and goals[nxt]==pos:nxt+=1
    assert nxt==m
    return m+2*n*m-len(lines),len(lines),edits,slides,len(walls)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for arg in ("gen","vis","base","candidate","csv"):
        p.add_argument(arg,type=Path)
    p.add_argument("--multistart",type=Path)
    a=p.parse_args()
    tag="cross-leg-v1" if a.multistart else "persistent-stops-v1"
    if a.multistart:a.multistart=a.multistart.resolve()
    gen,vis,base,candidate=(getattr(a,k).resolve() for k in ("gen","vis","base","candidate"))
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as f:
        work=Path(d)
        seed,ans=work/"seeds.txt",work/"answer.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","arm","score","turns","edits","slides","blocks_end","baseline","seconds","incumbent","checks"))
        for split in ("dev","holdout"):
            ids=[int.from_bytes(hashlib.sha256(f"ahc046/{tag}/{split}/{j}".encode()).digest()[:8],
                                "little") for j in range(30)]
            seed.write_text("".join(f"{s}\n" for s in ids))
            subprocess.run([gen,seed],cwd=work,check=True)
            for i,s in enumerate(ids):
                inp=work/"in"/f"{i:04d}.txt"
                data=inp.read_bytes()
                arms=[("static",base),("blocks",candidate)]
                if a.multistart:arms=[("blocks",base),("multi",a.multistart),("beam",candidate)]
                starts=[];incumbents=[]
                for label,binary in arms[::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    run=subprocess.run([binary],input=data,check=True,capture_output=True,timeout=2)
                    elapsed=time.monotonic()-start
                    result=replay(data,run.stdout)
                    found=re.fullmatch(rb"base=(\d+) turns=(\d+)(?: incumbent=(\d+) checks=(\d+))?\s*",run.stderr)
                    assert found and int(found[2])==result[1] and elapsed<2
                    starts.append(int(found[1]))
                    old=int(found[3]) if found[3] else result[1]
                    incumbents.append(old)
                    ans.write_bytes(run.stdout)
                    official=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert official.stdout.strip()==f"Score = {result[0]}",official.stdout
                    w.writerow((split,s,label,*result,starts[-1],f"{elapsed:.6f}",old,int(found[4]) if found[4] else 0))
                    f.flush()
                assert len(set(starts))==1
                if a.multistart:assert len(set(incumbents))==1

if __name__=="__main__":
    main()
