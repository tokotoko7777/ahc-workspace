"""Check orthogonal polygon legality and inclusive fish counts independently."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time

def validate(inp,out):
    n,*coords=map(int,inp.split())
    m,*values=map(int,out.split())
    assert 4<=m<=1000 and len(values)==2*m
    poly=list(zip(values[::2],values[1::2],strict=True))
    assert len(set(poly))==m
    assert all(0<=x<=100000 and 0<=y<=100000 for x,y in poly)
    edges=[]
    length=0
    for (x,y),(a,b) in zip(poly,poly[1:]+poly[:1]):
        assert (x==a) != (y==b)
        edges.append((min(x,a),max(x,a),min(y,b),max(y,b)))
        length+=abs(a-x)+abs(b-y)
    assert length<=400000
    for j,(x,a,y,b) in enumerate(edges):
        for k,(o,p,q,r) in enumerate(edges[:j]):
            x1,x2=max(x,o),min(a,p)
            y1,y2=max(y,q),min(b,r)
            if x1>x2 or y1>y2:continue
            assert (k==j-1 or (k==0 and j==m-1)) and x1==x2 and y1==y2
    horizontal=[(x,a,y) for x,a,y,b in edges if y==b]
    vertical=[(x,y,b) for x,a,y,b in edges if x==a]
    inside=[0,0]
    for i,(x,y) in enumerate(zip(coords[::2],coords[1::2],strict=True)):
        hit=any(y==s and a<=x<=b for a,b,s in horizontal)
        if not hit:
            crossings=0
            for q,a,b in vertical:
                if q==x and a<=y<=b:
                    hit=True
                    break
                if q>x and a<=y<b:crossings+=1
            hit=hit or crossings%2==1
        inside[i>=n]+=hit
    gain=inside[0]-inside[1]
    return max(0,gain+1),gain,length,m,*inside

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","vis","base","candidate","csv"):
        p.add_argument(k,type=Path)
    options=p.add_mutually_exclusive_group()
    options.add_argument("--overlap",action="store_true")
    options.add_argument("--resource",action="store_true")
    a=p.parse_args()
    tag="overlap-strips-v1" if a.overlap else "anchored-strips-v1"
    labels=("strips","overlap") if a.overlap else ("rect","strips")
    if a.resource:
        tag="overlap-resource-v1"
        labels=("extra-anchors","overlap")
    gen,vis,base,candidate=(getattr(a,k).resolve() for k in ("gen","vis","base","candidate"))
    with tempfile.TemporaryDirectory() as d,a.csv.open("w",newline="") as f:
        work=Path(d)
        seeds,ans=work/"seeds.txt",work/"ans.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","arm","score","gain","perimeter","vertices","positive","negative","seconds"))
        for split in ("dev","holdout"):
            ids=[int.from_bytes(hashlib.sha256(f"ahc039/{tag}/{split}/{i}".encode()).digest()[:8],
                               "little") for i in range(24)]
            seeds.write_text("".join(f"{x}\n" for x in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,seed in enumerate(ids):
                inp=work/"in"/f"{i:04d}.txt"
                data=inp.read_bytes()
                arms=list(zip(labels,(base,candidate)))
                same=[]
                for label,binary in arms[::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    x=subprocess.run([binary],input=data,check=True,capture_output=True,timeout=2)
                    elapsed=time.monotonic()-start
                    res=validate(data,x.stdout)
                    detail=re.fullmatch(rb"base=(\d+) gain=(\d+) perimeter=(\d+)\s*",x.stderr)
                    assert detail and tuple(map(int,detail.groups()[1:]))==res[1:3],(detail,res)
                    same.append(int(detail[1]))
                    ans.write_bytes(x.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,check=True,capture_output=True,text=True)
                    assert judge.stdout.strip()==f"Score = {res[0]}" and elapsed<2,(judge.stdout,res)
                    w.writerow((split,seed,label,*res,f"{elapsed:.6f}"))
                    f.flush()
                assert same[0]==same[1]

if __name__=="__main__":
    main()
