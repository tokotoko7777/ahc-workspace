"""Equal restarts on identical seeds with and without varied straight contraction order."""
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
    for k in ("gen","vis","base","variant","csv"):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,base,variant=(x.resolve() for x in (a.gen,a.vis,a.base,a.variant))
    with tempfile.TemporaryDirectory() as tmp,a.csv.open("w",newline="") as f:
        work=Path(tmp)
        seedfile,out=work/"seeds.txt",work/"out.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","arm","score","first","restarts","roots","seconds"))
        for split in ("dev","holdout"):
            seeds=[int.from_bytes(hashlib.sha256(f"ahc024/restart-v1/{split}/{i}".encode())
                                   .digest()[:8],"little") for i in range(24)]
            seedfile.write_text("".join(f"{s}\n" for s in seeds))
            subprocess.run([gen,seedfile],cwd=work,check=True)
            for i,seed in enumerate(seeds):
                inp=work/"in"/f"{i:04}.txt"
                case=inp.read_bytes()
                arms=[("same",base),("diverse",variant)]
                for name,cmd in (arms if i%2==0 else arms[::-1]):
                    start=time.monotonic()
                    run=subprocess.run([cmd],input=case,check=True,capture_output=True,timeout=2)
                    sec=time.monotonic()-start
                    assert sec<2
                    score=validate(case,run.stdout)
                    out.write_bytes(run.stdout)
                    vis_text=subprocess.run([vis,inp,out],cwd=work,capture_output=True,
                                            check=True,text=True).stdout.strip()
                    assert vis_text==f"Score = {score}",(seed,name,vis_text,score)
                    observed=re.fullmatch(rb"first=(\d+) restarts=(\d+) roots=(\d+)\s*",run.stderr)
                    assert observed and int(observed[1])<=score
                    w.writerow((split,seed,name,score,*map(int,observed.groups()),f"{sec:.6f}"))
                    f.flush()

if __name__=="__main__":
    main()
