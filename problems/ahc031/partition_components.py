"""Confirm alignment keeps the same strip layout and compare penalty components."""
import argparse
import json
from pathlib import Path
import subprocess

from benchmark import score


def components(inp, out):
    w, d, n, *areas = map(int, inp.split())
    values = tuple(map(int, out.split()))
    boxes = tuple(zip(*(values[i::4] for i in range(4)), strict=True))
    deficit = sum(100*max(0, area-(c-a)*(e-b))
                  for area,(a,b,c,e) in zip(areas,boxes,strict=True))
    strips = tuple((b,e) for a,b,c,e in boxes)
    return deficit, score(inp,out)-1-deficit, strips


def main():
    p=argparse.ArgumentParser()
    for name in ["cases","baseline","aligned"]:
        p.add_argument(name,type=Path)
    a=p.parse_args()
    totals=[[0,0],[0,0]]
    for case in sorted(a.cases.glob("*.txt")):
        inp=case.read_bytes()
        parts=[components(inp,subprocess.run([str(bin.resolve())],input=inp,
                   capture_output=True,timeout=3,check=True).stdout)
               for bin in (a.baseline,a.aligned)]
        before,after=parts
        if before[2]!=after[2] or after[0]>before[0] or after[1]>before[1]:
            raise ValueError(f"strip or cost invariant failed on {case}")
        for total,(area,wall,_) in zip(totals,parts,strict=True):
            total[0]+=area
            total[1]+=wall
    print(json.dumps({"baseline":totals[0],"aligned":totals[1]}))


if __name__=="__main__":
    main()
