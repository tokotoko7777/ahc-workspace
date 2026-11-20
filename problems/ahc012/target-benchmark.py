"""Compare same-start no addition, best axial addition, and any tested normal."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time
from benchmark import independent


def main():
    arg=argparse.ArgumentParser(description=__doc__)
    arg.add_argument("split",choices=("dev","holdout"))
    for name in ("gen","vis","solver","csv"):
        arg.add_argument(name,type=Path)
    a=arg.parse_args()
    gen,vis,solver=(x.resolve() for x in (a.gen,a.vis,a.solver))
    seeds=[int.from_bytes(hashlib.sha256(f"ahc012/target-v1/{a.split}/{i}".encode())
                          .digest()[:8],"little") for i in range(8)]
    with tempfile.TemporaryDirectory() as d, a.csv.open("w",newline="") as f:
        work=Path(d)
        seedfile,out=work/"seeds.txt",work/"out.txt"
        writer=csv.writer(f,lineterminator="\n")
        writer.writerow(("split","seed","frozen","axis","all","frozen_served",
                         "axis_served","all_served","axis_cut","all_cut","normals","seconds"))
        seedfile.write_text("".join(f"{s}\n" for s in seeds))
        subprocess.run([gen,seedfile],cwd=work,check=True)
        for i,seed in enumerate(seeds):
            inp=work/"in"/f"{i:04}.txt"
            data=inp.read_bytes()
            start=time.monotonic()
            p=subprocess.run([solver],input=data,check=True,capture_output=True,timeout=3)
            seconds=time.monotonic()-start
            assert seconds<3
            stats,axis,allcut=p.stderr.splitlines()
            m=dict((k,int(v)) for k,v in re.findall(r"(\w+)=(\d+)",stats.decode()))
            k,*lines=p.stdout.splitlines()
            assert len(lines)==int(k)
            if allcut!=b"-":
                assert lines.pop()==allcut
            scores=[]
            for mode,extra in (("frozen",b"-"),("axis",axis),("all",allcut)):
                plan=lines+([extra] if extra!=b"-" else [])
                result=b"\n".join([str(len(plan)).encode(),*plan])+b"\n"
                served,score,*_=independent(data,result)
                assert served==m[mode]
                out.write_bytes(result)
                official=subprocess.run([vis,inp,out],cwd=work,check=True,capture_output=True,
                                        text=True).stdout.strip()
                assert official==f"Score = {score}",(mode,seed,official,score)
                scores.append(score)
            assert scores[0]<=scores[1]<=scores[2]
            writer.writerow((a.split,seed,*scores,m["frozen"],m["axis"],m["all"],
                             axis.decode(),allcut.decode(),m["normals"],f"{seconds:.6f}"))
            f.flush()


if __name__=="__main__":
    main()
