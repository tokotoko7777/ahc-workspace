"""Equal-deletion, same-start suffix vs dependency-cascade tests."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time
from benchmark import validate


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument("split",choices=("dev","holdout"))
    for name in ("gen","vis","solver","csv"):
        ap.add_argument(name,type=Path)
    a=ap.parse_args()
    gen,vis,solver=(x.resolve() for x in (a.gen,a.vis,a.solver))
    seeds=[int.from_bytes(hashlib.sha256(f"ahc014/cascade-v1/{a.split}/{i}".encode())
                          .digest()[:8],"little") for i in range(6)]
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as f:
        work=Path(d)
        out,seedfile=work/"out.txt",work/"seeds.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","n","m","frozen","suffix","cascade",
                    "completed","deleted","preserved_after","seconds"))
        seedfile.write_text("".join(f"{s}\n" for s in seeds))
        subprocess.run([gen,seedfile],cwd=work,check=True)
        for i,seed in enumerate(seeds):
            inp=work/"in"/f"{i:04}.txt"
            data=inp.read_bytes()
            start=time.monotonic()
            p=subprocess.run([solver],input=data,check=True,capture_output=True,timeout=5)
            elapsed=time.monotonic()-start
            assert elapsed<5
            stats,*buf=p.stderr.splitlines()
            m=dict((k,int(v)) for k,v in re.findall(rb"(\w+)=(\d+)",stats))
            i1=1+int(buf[0])
            out1=b"\n".join(buf[:i1])+b"\n"
            out2=b"\n".join(buf[i1:])+b"\n"
            scores=[]
            for label,suite in zip((b"frozen",b"suffix",b"cascade"),(out1,out2,p.stdout)):
                weight,score,_,n,initial=validate(data,suite)
                assert weight==m[label]
                out.write_bytes(suite)
                check=subprocess.run([vis,inp,out],cwd=work,check=True,capture_output=True,text=True)
                assert check.stdout.strip()==f"Score = {score}",(label,seed,check.stdout,score)
                scores.append(score)
            assert scores[0]<=min(scores[1:])
            w.writerow((a.split,seed,n,initial,*scores,m[b"completed"],m[b"deleted"],
                         m[b"preserved_after"],f"{elapsed:.6f}"))
            f.flush()


if __name__=="__main__":
    main()
