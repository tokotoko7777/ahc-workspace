"""Official interactive tests and independent rectangle placement replay."""
import argparse
import csv
import hashlib
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import time

def replay(data,out):
    tokens=iter(map(int,data.split()))
    n,t,sigma=(next(tokens) for _ in range(3))
    obs=[(next(tokens),next(tokens)) for _ in range(n)]
    truth=[(next(tokens),next(tokens)) for _ in range(n)]
    errors=[(next(tokens),next(tokens)) for _ in range(t)]
    assert next(tokens,None) is None and len(obs)==n
    lines=iter(out.splitlines())
    outcomes=[]
    for rnd in range(t):
        count=int(next(lines))
        assert 0<=count<=n
        pos={}
        old=-1
        for _ in range(count):
            a,b,dir,c=next(lines).split()
            i,rot,anchor=int(a),int(b),int(c)
            assert old<i<n and rot in (0,1) and dir in (b"U",b"L")
            assert anchor==-1 or anchor in pos
            old=i
            w,h=truth[i][::(-1 if rot else 1)]
            if dir==b"U":
                x=0 if anchor==-1 else pos[anchor][2]
                colliders=(Y for X,y,R,Y in pos.values() if X<x+w and x<R)
                y=max(colliders,default=0)
            else:
                y=0 if anchor==-1 else pos[anchor][3]
                colliders=(R for X,Y,R,B in pos.values() if Y<y+h and y<B)
                x=max(colliders,default=0)
            if dir==b"U" and y>0:
                assert any(Y==y and X<x+w and R>x for X,_,R,Y in pos.values())
            if dir==b"L" and x>0:
                assert any(R==x and Y<y+h and B>y for _,Y,R,B in pos.values())
            assert all(x+w<=X or R<=x or y+h<=Y or B<=y for X,Y,R,B in pos.values())
            pos[i]=(x,y,x+w,y+h)
        omitted=sum(w+h for i,(w,h) in enumerate(truth) if i not in pos)
        W=max((v[2] for v in pos.values()),default=0)
        H=max((v[3] for v in pos.values()),default=0)
        outcomes.append((W+H+omitted,rnd,W,H,omitted))
    assert next(lines,None) is None
    out=min(outcomes)
    return (*out,n,t,sigma)

def run(tester,solver,data):
    start=time.monotonic()
    with subprocess.Popen([tester,solver],stdin=subprocess.PIPE,stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE,start_new_session=True) as process:
        try:
            out,err=process.communicate(data,timeout=3)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid,signal.SIGKILL)
            process.communicate()
            raise
        elapsed=time.monotonic()-start
        assert elapsed<3 and process.returncode==0
    return out,err,elapsed

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","tester","base","candidate","csv"):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,tester,base,candidate=(getattr(a,k).resolve() for k in ("gen","tester","base","candidate"))
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as stream:
        work=Path(d)
        seeds=work/"seeds.txt"
        w=csv.writer(stream,lineterminator="\n")
        w.writerow(("split","seed","arm","score","best_turn","W","H","omitted","N","T","sigma","seconds"))
        for split in ("dev","holdout"):
            ids=[int.from_bytes(hashlib.sha256(f"ahc040/mixed-slides-v1/{split}/{i}".encode()).digest()[:8],
                                "little") for i in range(32)]
            seeds.write_text("".join(f"{seed}\n" for seed in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                data=(work/"in"/f"{i:04d}.txt").read_bytes()
                arms=[("shelf",base),("mixed",candidate)]
                for label,binary in arms[::(-1 if i%2 else 1)]:
                    out,stderr,seconds=run(tester,binary,data)
                    score=replay(data,out)
                    assert stderr.strip()==f"Score = {score[0]}".encode(),stderr
                    w.writerow((split,seed,label,*score,f"{seconds:.6f}"))
                    stream.flush()

if __name__=="__main__":
    main()
