"""Paired flat, rolling, and rolling with bounded route probes."""
import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
from benchmark import run,validate


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ("gen","tester","flat","rolling","probes","csv"):
        p.add_argument(name,type=Path)
    a=p.parse_args()
    gen,tester,flat,rolling,probes=(x.resolve() for x in (a.gen,a.tester,a.flat,a.rolling,a.probes))
    with tempfile.TemporaryDirectory() as tmp,a.csv.open("w",newline="") as f:
        area=Path(tmp)
        seedfile=area/"seeds.txt"
        writer=csv.writer(f,lineterminator="\n")
        writer.writerow(("split","seed","arm","cost","commands","broken","water","houses",
                         "c","seconds","probes","probe_cost","unwatered_probe_cost"))
        for split in ("dev","holdout"):
            cases=[]
            for c in (1,2,4,8,16,32,64,128):
                for k in range(4):
                    s=int.from_bytes(hashlib.sha256(f"ahc018/route-v1/{split}/{c}/{k}".encode())
                                     .digest()[:8],"little")
                    cases.append((s,c))
            seedfile.write_text("".join(f"{seed} 0 0 {c}\n" for seed,c in cases))
            subprocess.run([gen,seedfile],cwd=area,check=True)
            for i,(seed,c) in enumerate(cases):
                case=(area/"in"/f"{i:04}.txt").read_bytes()
                arms=[("flat",flat),("rolling",rolling),("probes",probes)]
                arms=arms[i%3:]+arms[:i%3]
                for name,exe in arms:
                    raw,reported,seconds=run(tester,exe,case)
                    commands=[]
                    observed=[]
                    next_probe=None
                    for line in raw.splitlines():
                        if line.startswith(b"# route-probe "):
                            assert next_probe is None
                            next_probe=line[len(b"# route-probe "):]
                        else:
                            if next_probe is not None:
                                assert line==next_probe
                                observed.append(tuple(map(int,line.split())))
                                next_probe=None
                            commands.append(line)
                    assert next_probe is None
                    wet=set()
                    state=validate(case,b"\n".join(commands),wet)
                    n=int(case.split(None,1)[0])
                    total,nop,broken,water,houses,cost=state
                    assert c==cost
                    match=re.fullmatch(rb"Total Cost = (\d+)\s*",reported)
                    assert match and int(match[1])==total,(name,seed,reported)
                    assert len(observed)<=2*houses
                    assert len({(y,x) for y,x,_ in observed})==len(observed)
                    assert all(70<=p<=620 for _,_,p in observed)
                    expense=sum(c+p for _,_,p in observed)
                    assert expense<=min(1550*houses,2*houses*(620+c))
                    wasted=sum(c+p for y,x,p in observed if y*n+x not in wet)
                    assert name=="probes" or not observed
                    writer.writerow((split,seed,name,*state,f"{seconds:.6f}",
                                     len(observed),expense,wasted))
                    f.flush()


if __name__=="__main__":
    main()
