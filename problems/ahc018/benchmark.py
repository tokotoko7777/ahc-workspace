"""Run the official interactive tester and independently replay rock and water states."""
import argparse
import csv
import hashlib
import os
from pathlib import Path
import re
import signal
import subprocess
import tempfile
import time


def seeds(split):
    for cost in (1,2,4,8,16,32,64,128):
        for i in range(4):
            seed=int.from_bytes(hashlib.sha256(f"ahc018/sparse-v1/{split}/{cost}/{i}".encode())
                                 .digest()[:8],"little")
            yield seed,cost


def run(tester,binary,case):
    start=time.monotonic()
    with subprocess.Popen([tester,binary],stdin=subprocess.PIPE,stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE,start_new_session=True) as proc:
        try:
            stdout,stderr=proc.communicate(case,timeout=5)
            elapsed=time.monotonic()-start
            assert elapsed<5 and proc.returncode==0
            return stdout,stderr,elapsed
        except subprocess.TimeoutExpired:
            os.killpg(proc.pid,signal.SIGKILL)
            proc.communicate()
            raise


def validate(case,out):
    data=list(map(int,case.split()))
    n,w,k,cost=data[:4]
    hard=data[4:4+n*n]
    pos=data[4+n*n:]
    terminal=[y*n+x for y,x in zip(pos[::2],pos[1::2],strict=True)]
    sources=set(terminal[:w])
    houses=set(terminal[w:])
    assert len(hard)==n*n and len(houses)==k and len(sources)==w
    turns=out.splitlines()
    assert turns
    wet=set()
    total=progress=0
    def neighbors(p):
        y,x=divmod(p,n)
        for dy,dx in ((1,0),(-1,0),(0,1),(0,-1)):
            yy,xx=y+dy,x+dx
            if 0<=yy<n and 0<=xx<n:
                yield yy*n+xx
    broken_before=0
    for i,op in enumerate(turns):
        y,x,power=map(int,op.split())
        assert 0<=y<n and 0<=x<n and 1<=power<=5000
        tile=y*n+x
        assert hard[tile]>0
        hard[tile]-=power
        total+=cost+power
        if hard[tile]>0: continue
        broken_before+=1
        if tile in sources or any(a in wet for a in neighbors(tile)):
            q=[tile]
            wet.add(tile)
            for v in q:
                for u in neighbors(v):
                    if hard[u]<=0 and u not in wet:
                        q.append(u)
                        wet.add(u)
        if houses<=wet:
            assert i==len(turns)-1
            progress=1
            break
    assert progress
    return total,len(turns),broken_before,w,k,cost


def main():
    a=argparse.ArgumentParser(description=__doc__)
    for name in ("gen","tester","base","variant","csv"):
        a.add_argument(name,type=Path)
    args=a.parse_args()
    gen,tester,base,variant=(x.resolve() for x in (args.gen,args.tester,args.base,args.variant))
    with tempfile.TemporaryDirectory() as tmp,args.csv.open("w",newline="") as f:
        path=Path(tmp); seed_file=path/"seeds.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","arm","cost","commands","broken","water","houses",
                    "c","seconds"))
        for split in ("dev","holdout"):
            grouping=list(seeds(split))
            seed_file.write_text("".join(f"{s} 0 0 {c}\n" for s,c in grouping))
            subprocess.run([gen,seed_file],cwd=path,check=True)
            for i,(seed,c) in enumerate(grouping):
                case=(path/"in"/f"{i:04}.txt").read_bytes()
                modes=(("flat",base),("probes",variant))
                for arm,exe in (modes if i%2==0 else modes[::-1]):
                    reps,reported,seconds=run(tester,exe,case)
                    data=validate(case,reps)
                    assert c==data[-1]
                    stat=re.fullmatch(rb"Total Cost = (\d+)\s*",reported)
                    assert stat and int(stat[1])==data[0],(arm,seed,reported,data)
                    w.writerow((split,seed,arm,*data,f"{seconds:.6f}"))
                    f.flush()


if __name__=="__main__":
    main()
