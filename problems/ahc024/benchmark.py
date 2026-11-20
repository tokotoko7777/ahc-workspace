"""Score straight and wavy contraction from the same frozen board."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time


def topology(rows,colors):
    n=len(rows)
    pad=[[0]*(n+2)]+[[0,*r,0] for r in rows]+[[0]*(n+2)]
    edges=set()
    comps=[0]*(colors+1)
    seen=set()
    assert all(len(row)==n and all(0<=c<=colors for c in row) for row in rows)
    for y in range(n+2):
        for x in range(n+2):
            c=pad[y][x]
            for dy,dx in ((1,0),(0,1)):
                if y+dy<n+2 and x+dx<n+2:
                    d=pad[y+dy][x+dx]
                    if c!=d:
                        edges.add(tuple(sorted((c,d))))
            if (y,x) in seen:
                continue
            comps[c]+=1
            q=[(y,x)]
            seen.add((y,x))
            for i,j in q:
                for r,s in ((i+1,j),(i-1,j),(i,j+1),(i,j-1)):
                    if 0<=r<n+2 and 0<=s<n+2 and pad[r][s]==c and (r,s) not in seen:
                        seen.add((r,s))
                        q.append((r,s))
    assert all(k==1 for k in comps)
    return edges


def validate(case,out):
    seq=list(map(int,case.split()))
    n,m=seq[:2]
    rows=[seq[2+i*n:2+(i+1)*n] for i in range(n)]
    output=[list(map(int,row.split())) for row in out.splitlines()]
    assert len(output)==n
    assert topology(rows,m)==topology(output,m)
    return 1+sum(c==0 for row in output for c in row)


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for name in ("gen","vis","solver","csv"):
        ap.add_argument(name,type=Path)
    a=ap.parse_args()
    gen,vis,solver=(x.resolve() for x in (a.gen,a.vis,a.solver))
    with tempfile.TemporaryDirectory() as tmp,a.csv.open("w",newline="") as f:
        work=Path(tmp)
        seedfile,out=work/"seeds.txt",work/"out.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","straight","wavy","seams","frozen_area","seconds"))
        for split in ("dev","holdout"):
            seeds=[int.from_bytes(hashlib.sha256(f"ahc024/seams-v1/{split}/{i}".encode())
                                   .digest()[:8],"little") for i in range(24)]
            seedfile.write_text("".join(f"{s}\n" for s in seeds))
            subprocess.run([gen,seedfile],cwd=work,check=True)
            for i,seed in enumerate(seeds):
                inp=work/"in"/f"{i:04}.txt"
                case=inp.read_bytes()
                start=time.monotonic()
                run=subprocess.run([solver],input=case,check=True,capture_output=True,timeout=2)
                seconds=time.monotonic()-start
                assert seconds<2
                stats,*baseline=run.stderr.splitlines()
                match=re.fullmatch(rb"carved=(\d+) frozen_area=(\d+)",stats)
                assert match
                scores=[]
                for board in (b"\n".join(baseline)+b"\n",run.stdout):
                    score=validate(case,board)
                    out.write_bytes(board)
                    official=subprocess.run([vis,inp,out],cwd=work,check=True,capture_output=True,text=True)
                    assert official.stdout.strip()==f"Score = {score}",(seed,official.stdout,score)
                    scores.append(score)
                assert scores[0]<=scores[1]
                w.writerow((split,seed,*scores,*map(int,match.groups()),f"{seconds:.6f}"))
                f.flush()


if __name__=="__main__":
    main()
