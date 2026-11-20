"""Official stochastic dialogue; independently check exactly 100 guesses and absolute score."""
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


def setup(split,tag="degree-v1"):
    if tag=="model-v1" and split=="dev":
        m_set=(12,31,58,77,97)
        e_set=(0,3,8,15,21,28,35,40)
    elif tag=="model-v1":
        m_set=(10,35,56,83,100)
        e_set=(0,4,9,16,23,30,37,39)
    elif split=="dev":
        m_set=(10,28,55,80,100)
        e_set=(0,2,7,14,20,29,34,40)
    else:
        m_set=(10,34,60,86,100)
        e_set=(0,1,6,12,22,27,36,39)
    for m in m_set:
        for e in e_set:
            seed=int.from_bytes(hashlib.sha256(f"ahc016/{tag}/{split}/{m}/{e}".encode())
                                 .digest()[:8],"little")
            yield m,e,seed


def execute(tester,binary,case,limit):
    start=time.monotonic()
    with subprocess.Popen([tester,binary],stdin=subprocess.PIPE,stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE,start_new_session=True) as p:
        try:
            out,err=p.communicate(case,timeout=limit)
            assert p.returncode==0
        except subprocess.TimeoutExpired:
            os.killpg(p.pid,signal.SIGKILL)
            p.communicate()
            raise
    sec=time.monotonic()-start
    assert sec<limit
    return out,err,sec


def validate(case,out):
    m,e,*v=case.split()
    m=int(m)
    assert len(v)==101
    truth=list(map(int,v[:100]))
    n,*messages=out.splitlines()
    n=int(n)
    assert 4<=n<=100 and len(messages)==m+100
    for g in messages[:m]:
        assert len(g)==n*(n-1)//2 and set(g)<={ord("0"),ord("1")}
    guesses=list(map(int,messages[m:]))
    assert all(0<=t<m for t in guesses)
    mistake=sum(a!=b for a,b in zip(guesses,truth,strict=True))
    num=10**9*9**mistake
    denom=n*10**mistake
    score=(2*num+denom)//(2*denom)
    return n,mistake,score


def main():
    arg=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","tester","base","variant","csv"):
        arg.add_argument(k,type=Path)
    arg.add_argument("--tag",choices=("degree-v1","model-v1"),default="degree-v1")
    arg.add_argument("--limit",type=float,default=5.0)
    a=arg.parse_args()
    gen,tester,base,variant=(x.resolve() for x in (a.gen,a.tester,a.base,a.variant))
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as f:
        work=Path(d)
        seeds=work/"seed.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","m","eps","arm","n","errors","absolute","seconds"))
        for split in ("dev","holdout"):
            for k,(m,e,seed) in enumerate(setup(split,a.tag)):
                eps=f"{e/100:.2f}"
                seeds.write_text(f"{seed}\n")
                subprocess.run([gen,seeds,"--M",str(m),"--eps",eps],cwd=work,check=True)
                case=(work/"in/0000.txt").read_bytes()
                assert case.split()[:2]==[str(m).encode(),eps.encode()]
                modes=(("base",base),("model" if a.tag=="model-v1" else "degree",variant))
                for arm,binary in (modes if k%2==0 else modes[::-1]):
                    out,err,seconds=execute(tester,binary,case,a.limit)
                    n,E,value=validate(case,out)
                    check=re.fullmatch(rb"N = (\d+)\nE = (\d+)\n[ox]{100}\nScore = (\d+)\s*",err)
                    assert check is not None and tuple(map(int,check.groups()))==(n,E,value),err
                    w.writerow((split,seed,m,eps,arm,n,E,value,f"{seconds:.6f}"))
                    f.flush()


if __name__=="__main__":
    main()
