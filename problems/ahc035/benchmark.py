"""Run the official tester and replay all random masks independently."""
import argparse
import csv
import hashlib
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import time

def validate(data,output):
    t=iter(data.split())
    n,m,rounds=(int(next(t)) for _ in range(3))
    count=2*n*(n-1)
    values=[tuple(int(next(t)) for _ in range(m)) for _ in range(count)]
    maxima=[max(col) for col in zip(*values)]
    limit=sum(maxima)
    edges=[(r*n+c,r*n+c+1) for r in range(n) for c in range(n-1)]
    edges += [(r*n+c,(r+1)*n+c) for r in range(n-1) for c in range(n)]
    lines=[list(map(int,x.split())) for x in output.splitlines() if x and not x.startswith(b"#")]
    assert len(lines)==rounds*n and all(len(row)==n for row in lines)
    history=[]
    for turn in range(rounds):
        cells=sum(lines[turn*n:(turn+1)*n],[])
        assert len(set(cells))==n*n and all(0<=v<count for v in cells)
        children=[]
        for a,b in edges:
            mask=next(t)
            assert len(mask)==m and all(x in (48,49) for x in mask)
            profiles=values[cells[a]],values[cells[b]]
            children.append(tuple(profiles[x-48][j] for j,x in enumerate(mask)))
        history.append(max(sum(v) for v in children))
        values=children
    assert next(t,None) is None
    best=history[-1]
    remaining=sum(max(col) for col in zip(*values))
    return (2*10**6*best+limit)//(2*limit),best,limit,remaining

def run(test,binary,data):
    start=time.monotonic()
    with subprocess.Popen([test,binary],stdin=subprocess.PIPE,stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE,start_new_session=True) as process:
        try:
            out,err=process.communicate(data,timeout=2)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid,signal.SIGKILL)
            process.communicate()
            raise
        duration=time.monotonic()-start
        assert duration<2 and process.returncode==0
    return out,err,duration

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","tester","base","candidate","csv"):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,tester,base,candidate=(getattr(a,k).resolve() for k in ("gen","tester","base","candidate"))
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as stream:
        work=Path(d)
        file=work/"seeds.txt"
        w=csv.writer(stream,lineterminator="\n")
        w.writerow(("split","seed","arm","score","terminal_best","initial_ceiling","terminal_ceiling","seconds"))
        for split in ("dev","holdout"):
            seeds=[int.from_bytes(hashlib.sha256(f"ahc035/tail-selection-v1/{split}/{i}".encode()).digest()[:8],
                                "little") for i in range(36)]
            file.write_text("".join(f"{seed}\n" for seed in seeds))
            subprocess.run([gen,file],cwd=work,check=True)
            for i,seed in enumerate(seeds):
                data=(work/"in"/f"{i:04d}.txt").read_bytes()
                arms=[("degree",base),("tail",candidate)]
                for label,binary in arms[::(-1 if i%2 else 1)]:
                    out,err,seconds=run(tester,binary,data)
                    score=validate(data,out)
                    assert err.strip()==f"Score = {score[0]}".encode(),err
                    w.writerow((split,seed,label,*score,f"{seconds:.6f}"))
                    stream.flush()

if __name__=="__main__":
    main()
