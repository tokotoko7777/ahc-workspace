"""Both networks use exactly the same radii; independent exact connectivity and scoring."""
import argparse
import csv
import hashlib
from pathlib import Path
import subprocess
import tempfile
import time


def validate(case,output):
    data=list(map(int,case.split()))
    n,m,k=data[:3]
    vertices=list(zip(data[3:3+2*n:2],data[4:3+2*n:2],strict=True))
    raw=data[3+2*n:3+2*n+3*m]
    links=list(zip(raw[::3],raw[1::3],raw[2::3],strict=True))
    xy=data[3+2*n+3*m:]
    houses=list(zip(xy[::2],xy[1::2],strict=True))
    t=list(map(int,output.split()))
    assert len(t)==n+m and len(houses)==k
    p,b=t[:n],t[n:]
    assert all(0<=x<=5000 for x in p) and set(b)<={0,1}
    graph=[[] for _ in range(n)]
    link_cost=0
    for (a,z,w),keep in zip(links,b,strict=True):
        if keep:
            graph[a-1].append(z-1)
            graph[z-1].append(a-1)
            link_cost+=w
    seen={0}
    q=[0]
    for i in q:
        for j in graph[i]:
            if j not in seen:
                seen.add(j);q.append(j)
    emit=[(vertices[i][0],vertices[i][1],p[i]**2) for i in seen if p[i]>0]
    assert all(any((x-a)**2+(y-b)**2<=r for a,b,r in emit) for x,y in houses)
    radio_cost=sum(x*x for x in p)
    denominator=link_cost+radio_cost+10**7
    score=10**6+(2*10**14+denominator)//(2*denominator)
    return score,link_cost,radio_cost,len(emit)


def seeds(split):
    return [int.from_bytes(hashlib.sha256(f"ahc020/steiner-v1/{split}/{i}".encode())
                           .digest()[:8],"little") for i in range(16)]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ("gen","vis","solver","csv"):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,solver=(x.resolve() for x in (a.gen,a.vis,a.solver))
    with tempfile.TemporaryDirectory() as tmp,a.csv.open("w",newline="") as f:
        work=Path(tmp)
        seedfile,out=work/"seeds.txt",work/"out.txt"
        w=csv.writer(f,lineterminator="\n")
        w.writerow(("split","seed","arm","score","link_cost","radio_cost","stations","seconds"))
        for split in ("dev","holdout"):
            group=seeds(split)
            seedfile.write_text("".join(f"{s}\n" for s in group))
            subprocess.run([gen,seedfile],cwd=work,check=True)
            for i,seed in enumerate(group):
                path=work/"in"/f"{i:04}.txt"
                case=path.read_bytes()
                start=time.monotonic()
                run=subprocess.run([solver],input=case,check=True,capture_output=True,timeout=2)
                t=time.monotonic()-start
                assert t<2
                p,b=run.stdout.splitlines()
                base,=run.stderr.splitlines()
                scored=[]
                for arm,edges in (("mst",base),("steiner",b)):
                    ans=p+b"\n"+edges+b"\n"
                    x=validate(case,ans)
                    out.write_bytes(ans)
                    check=subprocess.run([vis,path,out],cwd=work,check=True,
                                         capture_output=True,text=True)
                    assert check.stdout.strip()==f"Score = {x[0]}",(seed,arm,check.stdout)
                    scored.append(x)
                    w.writerow((split,seed,arm,*x,f"{t:.6f}"))
                    f.flush()
                assert scored[0][2:]==scored[1][2:] and scored[1][1]<=scored[0][1]


if __name__=="__main__":
    main()
