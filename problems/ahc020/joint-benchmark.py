"""Compare SA objectives with identical root seed and fixed proposal/time budgets."""
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
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","vis","base","joint","csv"):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,base,joint=(x.resolve() for x in (a.gen,a.vis,a.base,a.joint))
    with tempfile.TemporaryDirectory() as tmp,a.csv.open("w",newline="") as f:
        work=Path(tmp)
        seedfile,out=work/"seeds.txt",work/"out.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","arm","score","link_cost","radio_cost","stations",
                    "station_mask","steps","legal","misses","seconds"))
        for split in ("dev","holdout"):
            seeds=[int.from_bytes(hashlib.sha256(f"ahc020/joint-v1/{split}/{i}".encode())
                                 .digest()[:8],"little") for i in range(16)]
            seedfile.write_text("".join(f"{s}\n" for s in seeds))
            subprocess.run([gen,seedfile],cwd=work,check=True)
            for i,seed in enumerate(seeds):
                inp=work/"in"/f"{i:04}.txt"
                data=inp.read_bytes()
                modes=[("base",base),("joint",joint)]
                for mode,binary in modes if i%2==0 else modes[::-1]:
                    start=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,check=True,timeout=2)
                    seconds=time.monotonic()-start
                    assert seconds<2
                    score,net,radio,stations=validate(data,run.stdout)
                    first=run.stdout.splitlines()[0]
                    mask="".join("1" if int(x)>0 else "0" for x in first.split())
                    stats=re.fullmatch(rb"steps=(\d+) legal=(\d+) misses=(\d+)\s*",run.stderr)
                    assert stats
                    out.write_bytes(run.stdout)
                    result=subprocess.run([vis,inp,out],cwd=work,check=True,capture_output=True,text=True)
                    assert result.stdout.strip()==f"Score = {score}",(mode,seed,result.stdout)
                    w.writerow((split,seed,mode,score,net,radio,stations,mask,
                                *map(int,stats.groups()),f"{seconds:.6f}"))
                    f.flush()


if __name__=="__main__":
    main()
