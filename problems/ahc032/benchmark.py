"""Independent modular replay plus paired official scoring of the same beam seed."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time

MOD=998244353

def replay(case,output):
    tokens=list(map(int,case.split()))
    n,m,k=tokens[:3]
    board=tokens[3:3+n*n]
    stamps=[tokens[3+n*n+9*i:3+n*n+9*(i+1)] for i in range(m)]
    count,*op=list(map(int,output.split()))
    assert 0<=count<=k and len(op)==3*count
    for mi,y,x in zip(op[::3],op[1::3],op[2::3],strict=True):
        assert 0<=mi<m and 0<=y<=n-3 and 0<=x<=n-3
        for dy in range(3):
            for dx in range(3):
                i=(y+dy)*n+x+dx
                board[i]=(board[i]+stamps[mi][3*dy+dx])%MOD
    return sum(board),count

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ("gen","vis","base","repair","csv"):
        p.add_argument(key,type=Path)
    a=p.parse_args()
    gen,vis,base,repair=(getattr(a,k).resolve() for k in ("gen","vis","base","repair"))
    with tempfile.TemporaryDirectory() as tmp,a.csv.open("w",newline="") as stream:
        work=Path(tmp)
        sf,out=work/"seeds.txt",work/"answer.txt"
        w=csv.writer(stream,lineterminator="\n")
        w.writerow(("split","seed","arm","score","moves","initial","beam_depth","accepted","seconds"))
        for split in ("dev","holdout"):
            seed=[int.from_bytes(hashlib.sha256(f"ahc032/coupled-stamp-v1/{split}/{i}".encode()).digest()[:8],
                                 "little") for i in range(16)]
            sf.write_text("".join(f"{j}\n" for j in seed))
            subprocess.run([gen,sf],cwd=work,check=True)
            for i,number in enumerate(seed):
                inp=work/"in"/f"{i:04d}.txt"
                case=inp.read_bytes()
                arms=[("base",base),("repair",repair)]
                paired={}
                for label,binary in arms[::1 if i%2==0 else -1]:
                    start=time.monotonic()
                    run=subprocess.run([binary],input=case,check=True,capture_output=True,timeout=2)
                    sec=time.monotonic()-start
                    score,count=replay(case,run.stdout)
                    out.write_bytes(run.stdout)
                    official=subprocess.run([vis,inp,out],cwd=work,check=True,capture_output=True,text=True)
                    assert official.stdout.strip()==f"Score = {score}",official.stdout
                    detail=re.fullmatch(rb"initial=(\d+) final=(\d+) depth=(\d+) accepted=(\d+)\s*",run.stderr)
                    assert detail
                    initial,final,depth,accepted=map(int,detail.groups())
                    assert score==final>=initial and sec<2
                    paired[label]=(initial,depth)
                    w.writerow((split,number,label,score,count,initial,depth,accepted,f"{sec:.6f}"))
                    stream.flush()
                assert paired["base"]==paired["repair"],paired

if __name__=="__main__":
    main()
