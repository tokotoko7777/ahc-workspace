"""Official scoring and separate cell-wise replay of all board shifts."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time

def validate(data,output):
    size,*grid=data.decode().split()
    n=int(size)
    board=[list(r) for r in grid]
    count_initial=sum(row.count("o") for row in board)
    steps=output.decode().splitlines()
    assert len(steps)<=4*n*n
    for line in steps:
        direction,num=line.split()
        k=int(num)
        assert 0<=k<n and direction in ("L","R","U","D")
        if direction=="L":board[k]=board[k][1:]+["."]
        elif direction=="R":board[k]=["."]+board[k][:-1]
        elif direction=="U":
            for i in range(n-1):board[i][k]=board[i+1][k]
            board[-1][k]="."
        else:
            for i in range(n-1,0,-1):board[i][k]=board[i-1][k]
            board[0][k]="."
    left=sum(row.count("x") for row in board)
    lost=count_initial-sum(row.count("o") for row in board)
    score=4*n*n-n*(left+lost) if left or lost else 8*n*n-len(steps)
    return score,len(steps),left,lost

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","vis","base","candidate","csv"):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,base,candidate=(getattr(a,k).resolve() for k in ("gen","vis","base","candidate"))
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as file:
        work=Path(d)
        seeds,output=work/"seeds.txt",work/"answer.txt"
        w=csv.writer(file,lineterminator="\n")
        w.writerow(("split","seed","arm","score","turns","left","lost","start","seconds"))
        for split in ("dev","holdout"):
            ids=[int.from_bytes(hashlib.sha256(f"ahc042/partial-restore-v1/{split}/{i}".encode()).digest()[:8],
                                "little") for i in range(32)]
            seeds.write_text("".join(f"{x}\n" for x in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/"in"/f"{i:04d}.txt"
                data=inp.read_bytes()
                arms=[("restore",base),("partial",candidate)]
                starts=[]
                for name,binary in arms[::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    run=subprocess.run([binary],input=data,check=True,capture_output=True,timeout=2)
                    seconds=time.monotonic()-start
                    result=validate(data,run.stdout)
                    m=re.fullmatch(rb"base=(\d+) turns=(\d+)\s*",run.stderr)
                    assert m and int(m[2])==result[1] and result[2:]==(0,0)
                    starts.append(int(m[1]))
                    output.write_bytes(run.stdout)
                    check=subprocess.run([vis,inp,output],cwd=work,check=True,capture_output=True,text=True)
                    assert check.stdout.strip()==f"Score = {result[0]}" and seconds<2
                    w.writerow((split,seed,name,*result,int(m[1]),f"{seconds:.6f}"))
                    file.flush()
                assert starts[0]==starts[1]

if __name__=="__main__":
    main()
