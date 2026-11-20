"""Official scoring plus independent lockstep and cellwise replay."""
import argparse
from collections import Counter
import csv
import hashlib
from pathlib import Path
import re
import select
import subprocess
import tempfile
import time

N=10
T=N*N


def gamecase(payload):
    a=list(map(int,payload.split()))
    assert len(a)==2*T
    return a[:T],a[T:]


def lockstep(binary,flavors,ranks):
    start=time.monotonic()
    with subprocess.Popen([binary],stdin=subprocess.PIPE,stdout=subprocess.PIPE,
                          stderr=subprocess.DEVNULL,bufsize=0) as p:
        try:
            assert p.stdin and p.stdout
            p.stdin.write((" ".join(map(str,flavors))+"\n").encode())
            output=[]
            for rank in ranks:
                p.stdin.write(f"{rank}\n".encode())
                raw=b""
                while not raw.endswith(b"\n"):
                    left=2-(time.monotonic()-start)
                    if left<=0 or not select.select([p.stdout],[],[],left)[0]:
                        raise TimeoutError("tilt response not flushed within wall budget")
                    block=p.stdout.read(4)
                    assert block,"solver terminated before the turn response"
                    raw+=block
                    assert len(raw)<=2
                assert raw in (b"F\n",b"B\n",b"L\n",b"R\n")
                output.append(raw)
            p.stdin.close()
            assert p.wait(timeout=max(0.001,2-(time.monotonic()-start)))==0
            assert p.stdout.read()==b""
            elapsed=time.monotonic()-start
            assert elapsed<2
            return b"".join(output),elapsed
        finally:
            if p.poll() is None:
                p.kill()
                p.wait()


def independent(flavors,ranks,answer):
    moves=answer.decode().splitlines()
    assert len(moves)==T
    b=[[0]*N for _ in range(N)]
    for t,(f,p,d) in enumerate(zip(flavors,ranks,moves,strict=True)):
        free=[(r,c) for r in range(N) for c in range(N) if b[r][c]==0]
        assert len(free)==T-t and 1<=p<=len(free) and d in ["F","B","L","R"]
        r,c=free[p-1]
        b[r][c]=f
        vertical=d in ("F","B")
        rows=list(zip(*b,strict=True)) if vertical else b
        packed=[]
        for row in rows:
            solids=[q for q in row if q]
            pad=[0]*(N-len(solids))
            packed.append(solids+pad if d in ("F","L") else pad+solids)
        b=[list(row) for row in zip(*packed,strict=True)] if vertical else packed
    seen=set()
    squares=0
    largest=[0,0,0]
    for r in range(N):
        for c in range(N):
            if (r,c) in seen:
                continue
            val=b[r][c]
            q=[(r,c)]
            seen.add((r,c))
            for i,j in q:
                for x,y in ((i+1,j),(i-1,j),(i,j+1),(i,j-1)):
                    if 0<=x<N and 0<=y<N and (x,y) not in seen and b[x][y]==val:
                        q.append((x,y))
                        seen.add((x,y))
            squares+=len(q)**2
            largest[val-1]=max(largest[val-1],len(q))
    denom=sum(n*n for n in Counter(flavors).values())
    return (2_000_000*squares+denom)//(2*denom),squares,largest


def seeds(split):
    return [int.from_bytes(hashlib.sha256(f"ahc015/rollout-v1/{split}/{i}".encode())
                           .digest()[:8],"little") for i in range(24)]


def main():
    arg=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","vis","base","rollout","csv"):
        arg.add_argument(k,type=Path)
    a=arg.parse_args()
    gen,vis,base,rollout=(x.resolve() for x in (a.gen,a.vis,a.base,a.rollout))
    with tempfile.TemporaryDirectory() as d, a.csv.open("w",newline="") as f:
        work=Path(d)
        inp_seeds,out=work/"seeds.txt",work/"out.txt"
        write=csv.writer(f,lineterminator="\n")
        write.writerow(("split","seed","arm","score","squares","max1","max2","max3","seconds"))
        for split in ("dev","holdout"):
            inp_seeds.write_text("".join(f"{s}\n" for s in seeds(split)))
            subprocess.run([gen,inp_seeds],cwd=work,check=True)
            for i,s in enumerate(seeds(split)):
                inp=work/"in"/f"{i:04}.txt"
                flavors,ranks=gamecase(inp.read_bytes())
                arms=[("base",base),("rollout",rollout)]
                for arm,binary in (arms if i%2==0 else arms[::-1]):
                    answer,seconds=lockstep(binary,flavors,ranks)
                    score,squares,largest=independent(flavors,ranks,answer)
                    out.write_bytes(answer)
                    check=subprocess.run([vis,inp,out],cwd=work,check=True,capture_output=True,
                                         text=True).stdout.strip()
                    assert check==f"Score = {score}",(split,s,arm,check,score)
                    write.writerow((split,s,arm,score,squares,*largest,f"{seconds:.6f}"))
                    f.flush()


if __name__=="__main__":
    main()
