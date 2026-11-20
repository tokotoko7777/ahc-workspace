"""Compare inherited rollout and both beam variants using exact LIFO replay."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time


def validate(case,out):
    data=list(map(int,case.split()))
    n,m=data[:2]
    length=n//m
    piles=[data[2+i*length:2+(i+1)*length] for i in range(m)]
    op=[tuple(map(int,z.split())) for z in out.splitlines()]
    assert len(op)<=5000
    goal,energy,moves=1,0,0
    for v,dst in op:
        assert goal<=v<=n and 0<=dst<=m
        position=next(((i,s.index(v)) for i,s in enumerate(piles) if v in s),None)
        assert position is not None
        p,h=position
        if dst==0:
            assert v==goal and h==len(piles[p])-1
            piles[p].pop()
            goal+=1
        else:
            assert dst-1!=p
            chunk=piles[p][h:]
            energy+=len(chunk)+1
            moves+=1
            piles[dst-1].extend(chunk)
            del piles[p][h:]
    assert goal==n+1 and all(not s for s in piles)
    return max(1,10000-energy),energy,moves,len(op)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","vis","old","bulk","split","csv"):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,old,bulk,split=(x.resolve() for x in (a.gen,a.vis,a.old,a.bulk,a.split))
    with tempfile.TemporaryDirectory() as tmp,a.csv.open("w",newline="") as f:
        work=Path(tmp)
        seedfile,out=work/"seeds.txt",work/"out.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","arm","score","energy","moves","commands","depth","seconds"))
        for r in ("dev","holdout"):
            seeds=[int.from_bytes(hashlib.sha256(f"ahc026/split-beam-v1/{r}/{i}".encode())
                                   .digest()[:8],"little") for i in range(16)]
            seedfile.write_text("".join(f"{s}\n" for s in seeds))
            subprocess.run([gen,seedfile],cwd=work,check=True)
            for i,seed in enumerate(seeds):
                inp=work/"in"/f"{i:04}.txt"
                case=inp.read_bytes()
                arms=[("inherited",old),("bulk",bulk),("split",split)]
                j=i%3
                for label,binary in arms[j:]+arms[:j]:
                    start=time.monotonic()
                    run=subprocess.run([binary],input=case,check=True,capture_output=True,timeout=2)
                    sec=time.monotonic()-start
                    assert sec<2
                    answer=validate(case,run.stdout)
                    out.write_bytes(run.stdout)
                    official=subprocess.run([vis,inp,out],cwd=work,check=True,capture_output=True,text=True)
                    assert official.stdout.strip()==f"Score = {answer[0]}",(r,seed,label,official.stdout)
                    depth=200
                    if label!="inherited":
                        trace=re.fullmatch(rb"cost=(\d+) depth=(\d+)\s*",run.stderr)
                        assert trace and int(trace[1])==answer[1]
                        depth=int(trace[2])
                    w.writerow((r,seed,label,*answer,depth,f"{sec:.6f}"))
                    f.flush()


if __name__=="__main__":
    main()
