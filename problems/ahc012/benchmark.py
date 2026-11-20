"""Paired development and untouched generator cases; official + integer scorer."""
import argparse
from collections import Counter
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time


def independent(case, output):
    data = list(map(int, case.split()))
    n, budget, *a = data[:12]
    points = list(zip(data[12::2], data[13::2], strict=True))
    assert len(points) == n
    draw = list(map(int, output.split()))
    k = draw[0]
    assert 0 <= k <= budget and len(draw) == 1+4*k
    cuts = list(zip(draw[1::4],draw[2::4],draw[3::4],draw[4::4],strict=True))
    assert all((x,y) != (p,q) and max(map(abs, (x,y,p,q))) <= 10**9
               for x,y,p,q in cuts)
    # General 64-bit integer cross product, independent of family/bin layout.
    lines = [(y-q,p-x,(p-x)*y-(q-y)*x) for x,y,p,q in cuts]
    counts = Counter()
    for x,y in points:
        mask = 0
        for u,v,w in lines:
            z = u*x+v*y-w
            if z == 0:
                break
            mask = (mask << 1) | (z > 0)
        else:
            counts[mask]+=1
    freq = Counter(counts.values())
    matched = sum(min(a[d-1],freq[d]) for d in range(1,11))
    total = sum(a)
    return matched, (2_000_000*matched + total)//(2*total), k, total


def seeds(split):
    return [int.from_bytes(hashlib.sha256(f"ahc012/grid-families-v1/{split}/{i}".encode())
                           .digest()[:8], "little") for i in range(8)]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    for name in ("gen","vis","base","variant","csv"):
        ap.add_argument(name,type=Path)
    args = ap.parse_args()
    gen, vis, base, variant = (x.resolve() for x in (args.gen,args.vis,args.base,args.variant))
    with tempfile.TemporaryDirectory() as d, args.csv.open("w",newline="") as f:
        work = Path(d)
        out, seedfile = work/"out.txt",work/"seeds.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","arm","score","served","target","cuts","diagonal","seconds"))
        for split in ("dev","holdout"):
            seedfile.write_text("".join(f"{s}\n" for s in seeds(split)))
            subprocess.run([gen,seedfile],cwd=work,check=True)
            for i,seed in enumerate(seeds(split)):
                inp=work/"in"/f"{i:04}.txt"
                payload=inp.read_bytes()
                arms=[("base",base),("portfolio",variant)]
                for arm,command in (arms if i%2==0 else arms[::-1]):
                    start=time.monotonic()
                    p=subprocess.run([command],input=payload,check=True,capture_output=True,timeout=3)
                    elapsed=time.monotonic()-start
                    assert elapsed<3
                    out.write_bytes(p.stdout)
                    matched,score,cuts,total=independent(payload,p.stdout)
                    match=re.fullmatch(rb"served=(\d+) diagonal=(\d+)\s*",p.stderr)
                    assert match and int(match[1])==matched
                    official=subprocess.run([vis,inp,out],cwd=work,check=True,capture_output=True,text=True).stdout.strip()
                    assert official==f"Score = {score}",(arm,seed,official,score)
                    w.writerow((split,seed,arm,score,matched,total,cuts,int(match[2]),f"{elapsed:.6f}"))
                    f.flush()


if __name__=="__main__":
    main()
