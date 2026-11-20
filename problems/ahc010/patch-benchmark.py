"""Compare frozen, one-tile, and joint 2x2 candidate rotations on identical starts."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time

from benchmark import independent_score


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("split", choices=("dev", "holdout"))
    for name in ("gen", "vis", "solver", "csv"):
        p.add_argument(name, type=Path)
    args = p.parse_args()
    gen, vis, solver = (x.resolve() for x in (args.gen, args.vis, args.solver))
    seeds = [int.from_bytes(hashlib.sha256(
        f"ahc010/patch-v1/{args.split}/{i}".encode()).digest()[:8], "little") for i in range(8)]
    metrics = ("l", "r", "sl", "sr", "jl", "jr", "lp", "rp")
    with tempfile.TemporaryDirectory() as d, args.csv.open("w", newline="") as f:
        work = Path(d)
        seedfile, output = work / "seeds.txt", work / "out.txt"
        w = csv.writer(f, lineterminator="\n")
        w.writerow(("split", "seed", "frozen", "single", "joint", *metrics, "seconds"))
        seedfile.write_text("".join(f"{s}\n" for s in seeds))
        subprocess.run([gen,seedfile],cwd=work,check=True)
        for i, seed in enumerate(seeds):
            inp=work/"in"/f"{i:04}.txt"
            payload=inp.read_bytes()
            start=time.monotonic()
            run=subprocess.run([solver],input=payload,capture_output=True,check=True,timeout=2)
            seconds=time.monotonic()-start
            assert seconds<2
            stats, frozen, single=run.stderr.splitlines()
            data=dict((k,int(v)) for k,v in re.findall(r"(\w+)=(\d+)",stats.decode()))
            scores=[]
            for ans, l, r in zip((frozen,single,run.stdout),("l","sl","jl"),("r","sr","jr")):
                score, _=independent_score(payload,ans)
                assert score>=data[l]*data[r]
                output.write_bytes(ans.strip()+b"\n")
                actual=subprocess.run([vis,inp,output],cwd=work,check=True,capture_output=True,
                                      text=True).stdout.strip()
                assert actual==f"Score = {score}",(i,actual,score)
                scores.append(score)
            w.writerow((args.split,seed,*scores,*(data[k] for k in metrics),f"{seconds:.6f}"))
            f.flush()


if __name__=="__main__":
    main()
