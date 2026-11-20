"""Paired exact completion of one whole/split edit on frozen incumbent states."""
import argparse
from pathlib import Path
import csv
import hashlib
import re
import subprocess
import tempfile
import time
from benchmark import validate

p=argparse.ArgumentParser(description=__doc__)
for name in ("gen","vis","binary","csv"):
    p.add_argument(name,type=Path)
p.add_argument("--splits",nargs="+",default=["dev","holdout"],choices=["dev","holdout"])
a=p.parse_args()
gen,vis,binary=(getattr(a,n).resolve() for n in ("gen","vis","binary"))
with tempfile.TemporaryDirectory() as directory, a.csv.open("w",newline="") as stream:
    tmp=Path(directory)
    out=tmp/"answer.txt"
    w=csv.writer(stream,lineterminator="\n")
    w.writerow(("split","seed","arm","score","energy","incumbent","snapshots","evaluations","edit","seconds"))
    for split in a.splits:
        seeds=[int.from_bytes(hashlib.sha256(f"ahc026/frozen-snapshot-v1/{split}/{i}".encode()).digest()[:8],
                              "little") for i in range(20)]
        seedfile=tmp/"seeds.txt"
        seedfile.write_text("".join(f"{x}\n" for x in seeds))
        subprocess.run([gen,seedfile],cwd=tmp,check=True)
        for i,seed in enumerate(seeds):
            inp=tmp/"in"/f"{i:04d}.txt"
            case=inp.read_bytes()
            paired=[]
            for arm in (("whole","split") if i%2==0 else ("split","whole")):
                start=time.monotonic()
                r=subprocess.run([binary,arm],input=case,capture_output=True,timeout=2,check=True)
                seconds=time.monotonic()-start
                score,energy,_,_=validate(case,r.stdout)
                out.write_bytes(r.stdout)
                official=subprocess.run([vis,inp,out],cwd=tmp,check=True,capture_output=True,text=True)
                assert official.stdout.strip()==f"Score = {score}" and seconds<2
                device=re.fullmatch(rb"incumbent=(\d+) energy=(\d+) snapshots=(\d+) evaluations=(\d+) edit=(\d+)\s*",r.stderr)
                assert device,r.stderr
                old,current,snaps,evaluations,edit=map(int,device.groups())
                assert current==energy<=old and evaluations==36*snaps
                assert (edit==0)==(current==old)
                paired.append((old,snaps,evaluations))
                w.writerow((split,seed,arm,score,energy,old,snaps,evaluations,edit,f"{seconds:.6f}"))
                stream.flush()
            assert paired[0]==paired[1],paired
