"""Official scorer plus independent integer unit-edge replay for paired boards."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time


def validate(case, output):
    v=list(map(int,case.split()))
    n,m=v[:2]
    marks=set(zip(v[2::2],v[3::2],strict=True))
    assert len(marks)==m
    result=list(map(int,output.split()))
    k=result[0]
    assert k>=0 and len(result)==1+8*k
    edges=set()
    for j in range(k):
        w=result[1+8*j:9+8*j]
        corners=list(zip(w[::2],w[1::2],strict=True))
        assert len(set(corners))==4
        p,a,b,c=corners
        assert all(0<=x<n and 0<=y<n for x,y in corners)
        assert p not in marks and all(q in marks for q in (a,b,c))
        u=(a[0]-p[0],a[1]-p[1])
        v=(c[0]-p[0],c[1]-p[1])
        assert u[0]*v[0]+u[1]*v[1]==0
        assert b==(p[0]+u[0]+v[0],p[1]+u[1]+v[1])
        for (x,y),(x1,y1) in zip(corners,corners[1:]+[p],strict=True):
            dx,dy=x1-x,y1-y
            length=max(abs(dx),abs(dy))
            assert length>0 and abs(dx) in (0,length) and abs(dy) in (0,length)
            sx,sy=dx//length,dy//length
            for i in range(length):
                a=(x+sx*i,y+sy*i)
                b=(x+sx*(i+1),y+sy*(i+1))
                edge=tuple(sorted((a,b)))
                assert i==0 or a not in marks
                assert edge not in edges
                edges.add(edge)
        marks.add(p)
    c=(n-1)//2
    total=sum((x-c)**2+(y-c)**2+1 for x,y in marks)
    all_weight=sum((x-c)**2+(y-c)**2+1 for x in range(n) for y in range(n))
    num=10**6*n*n*total
    den=m*all_weight
    return total,(2*num+den)//(2*den),k,n,m


def seeds(split):
    return [int.from_bytes(hashlib.sha256(f"ahc014/regrowth-v1/{split}/{i}".encode())
                           .digest()[:8],"little") for i in range(6)]


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for name in ("gen","vis","base","variant","csv"):
        ap.add_argument(name,type=Path)
    args=ap.parse_args()
    gen,vis,base,variant=(x.resolve() for x in (args.gen,args.vis,args.base,args.variant))
    with tempfile.TemporaryDirectory() as d, args.csv.open("w",newline="") as f:
        work=Path(d)
        out,seedfile=work/"out.txt",work/"seeds.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","arm","score","weight","moves","n","m",
                    "full","rewind","seconds"))
        for split in ("dev","holdout"):
            seedfile.write_text("".join(f"{s}\n" for s in seeds(split)))
            subprocess.run([gen,seedfile],cwd=work,check=True)
            for i,seed in enumerate(seeds(split)):
                inp=work/"in"/f"{i:04}.txt"
                data=inp.read_bytes()
                modes=[("base",base),("regrowth",variant)]
                for mode,cmd in (modes if i%2==0 else modes[::-1]):
                    start=time.monotonic()
                    run=subprocess.run([cmd],input=data,capture_output=True,check=True,timeout=5)
                    seconds=time.monotonic()-start
                    assert seconds<5
                    weight,score,k,n,m=validate(data,run.stdout)
                    stats=re.fullmatch(rb"weight=(\d+) full=(\d+) rewind=(\d+)\s*",run.stderr)
                    assert stats and int(stats[1])==weight
                    out.write_bytes(run.stdout)
                    check=subprocess.run([vis,inp,out],cwd=work,check=True,
                                         capture_output=True,text=True).stdout.strip()
                    assert check==f"Score = {score}",(seed,mode,score,check)
                    w.writerow((split,seed,mode,score,weight,k,n,m,int(stats[2]),
                               int(stats[3]),f"{seconds:.6f}"))
                    f.flush()


if __name__=="__main__":
    main()
