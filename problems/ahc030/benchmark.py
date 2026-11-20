"""Official interactive tester plus independently replayed query transcripts."""
import argparse
import csv
import hashlib
import math
from pathlib import Path
import re
import subprocess
import tempfile
import time

def replay(case, transcript):
    data=iter(case.split())
    n,m,eps=int(next(data)),int(next(data)),float(next(data))
    shape=[[(int(next(data)),int(next(data))) for _ in range(int(next(data)))]
           for _ in range(m)]
    offsets=[(int(next(data)),int(next(data))) for _ in range(m)]
    field={(y,x):int(next(data)) for y in range(n) for x in range(n)}
    check=dict.fromkeys(field,0)
    for ps,(y,x) in zip(shape,offsets):
        for dy,dx in ps:
            check[y+dy,x+dx]+=1
    assert field==check
    support={p for p,v in field.items() if v>0}
    lines=[x.split() for x in transcript.splitlines() if x and not x.startswith("#")]
    assert 0<len(lines)<=2*n*n
    price,drills,surveys,guesses=0.0,0,0,0
    for t,(kind,nc,*points) in enumerate(lines):
        k=int(nc)
        assert len(points)==2*k and 1<=k<=n*n
        cells=set(zip(map(int,points[::2]),map(int,points[1::2])))
        assert len(cells)==k and cells<=field.keys()
        if kind=="a":
            guesses+=1
            if cells==support:
                assert t==len(lines)-1
                return (math.floor(1e6*max(price,1.0/n)+0.5),drills,surveys,guesses,n,m,eps)
            price+=1
        else:
            assert kind=="q"
            if k==1:
                drills+=1
                price+=1
            else:
                surveys+=1
                price+=1/math.sqrt(k)
    raise AssertionError("no correct answer")

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ["gen","tester","scan","active","csv"]:
        parser.add_argument(name,type=Path)
    args=parser.parse_args()
    gen,tester,scan,active=(getattr(args,key).resolve() for key in ("gen","tester","scan","active"))
    with tempfile.TemporaryDirectory() as tmp,args.csv.open("w",newline="") as stream:
        work=Path(tmp)
        seedfile=work/"seeds.txt"
        writer=csv.writer(stream,lineterminator="\n")
        writer.writerow(("split","seed","arm","score","drills","surveys","guesses","n","m","eps","seconds"))
        for split in ("dev","holdout"):
            seeds=[int.from_bytes(hashlib.sha256(f"ahc030/exact-fields-v1/{split}/{i}".encode()).digest()[:8],
                                  "little") for i in range(40)]
            seedfile.write_text("".join(f"{n}\n" for n in seeds))
            subprocess.run([gen,seedfile],cwd=work,check=True)
            for i,seed in enumerate(seeds):
                case=(work/"in"/f"{i:04d}.txt").read_text()
                arms=[("scan",scan),("active",active)]
                for label,binary in arms[::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    run=subprocess.run([tester,binary],input=case,check=True,capture_output=True,
                                       timeout=3,text=True)
                    sec=time.monotonic()-start
                    score,*diagnostics=replay(case,run.stdout)
                    assert re.fullmatch(fr"Score = {score}\s*",run.stderr),(score,run.stderr)
                    assert sec<3
                    writer.writerow((split,seed,label,score,*diagnostics,f"{sec:.6f}"))
                    stream.flush()

if __name__=="__main__":
    main()
