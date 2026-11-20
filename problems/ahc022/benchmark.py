"""Official interactive replay plus exact independent integer objective checks."""
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


def run(tester,binary,case):
    start=time.monotonic()
    with subprocess.Popen([tester,binary],stdin=subprocess.PIPE,stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE,start_new_session=True) as p:
        try:
            out,err=p.communicate(case,timeout=4)
            elapsed=time.monotonic()-start
            assert p.returncode==0 and elapsed<4
            return out,err,elapsed
        except subprocess.TimeoutExpired:
            os.killpg(p.pid,signal.SIGKILL)
            p.communicate()
            raise


def validate(case,out):
    inp=list(map(int,case.split()))
    l,n,s=inp[:3]
    truth=inp[3+2*n:3+3*n]
    assert len(inp)==3+3*n+10000
    ans=out.splitlines()
    assert len(ans)>=l+n+1
    grid=[list(map(int,row.split())) for row in ans[:l]]
    assert all(len(row)==l and all(0<=v<=1000 for v in row) for row in grid)
    assert ans[-n-1].split()==[b"-1",b"-1",b"-1"]
    measurements=ans[l:-n-1]
    assert len(measurements)<=10000
    measure=0
    for q in measurements:
        i,y,x=map(int,q.split())
        assert 0<=i<n and -l<y<l and -l<x<l
        measure+=100*(10+abs(y)+abs(x))
    guess=list(map(int,ans[-n:]))
    assert sorted(guess)==list(range(n))
    errors=sum(i!=j for i,j in zip(truth,guess,strict=True))
    placement=sum((grid[y][x]-grid[(y+1)%l][x])**2+
                  (grid[y][x]-grid[y][(x+1)%l])**2
                  for y in range(l) for x in range(l))
    numer=10**14*4**errors
    denom=(100000+placement+measure)*5**errors
    return (numer+denom-1)//denom,errors,placement,measure,len(measurements),l,n,s


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for name in ("gen","tester","base","candidate","csv"):
        ap.add_argument(name,type=Path)
    a=ap.parse_args()
    gen,tester,base,candidate=(x.resolve() for x in (a.gen,a.tester,a.base,a.candidate))
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as f:
        work=Path(d)
        seedfile=work/"seeds.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","arm","score","errors","placement","measure_cost",
                    "queries","l","n","s","seconds"))
        for split in ("dev","holdout"):
            seeds=[(int.from_bytes(hashlib.sha256(f"ahc022/stencil-v1/{split}/{i}".encode())
                                   .digest()[:8],"little"),i*i) for i in range(1,31)]
            seedfile.write_text("".join(f"{seed} 0 0 {s}\n" for seed,s in seeds))
            subprocess.run([gen,seedfile],cwd=work,check=True)
            for i,(seed,s) in enumerate(seeds):
                inp=work/"in"/f"{i:04}.txt"
                case=inp.read_bytes()
                arms=[("scalar",base),("stencil",candidate)]
                for label,cmd in (arms if i%2==0 else arms[::-1]):
                    out,observed,elapsed=run(tester,cmd,case)
                    result=validate(case,out)
                    m=re.fullmatch(rb"Score = (\d+)\nNumber of wrong answers = (\d+)"
                                  rb"\nPlacement cost = (\d+)\nMeasurement cost = (\d+)"
                                  rb"\nMeasurement count = (\d+)\s*",observed)
                    assert m and tuple(map(int,m.groups()))==result[:5],(split,seed,label,observed,result)
                    assert s==result[-1]
                    w.writerow((split,seed,label,*result,f"{elapsed:.6f}"))
                    f.flush()


if __name__=="__main__":
    main()
