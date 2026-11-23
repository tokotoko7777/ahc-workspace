"""Exact capture-before-first-exit probability, no animal outside the pen."""
import json
from fractions import Fraction as F
from collections import defaultdict
from audit import m, safe_starts

def capture_prob(body, gates, safe, stride):
    body=set(body); gates=set(gates)
    stages=sorted(body-safe)
    n=len(stages)
    if not n:return {p:F(1) for p in body}
    index={p:i for i,p in enumerate(stages)}
    matrix=[[F(0)]*(n+1) for _ in range(n)]
    for i,p in enumerate(stages):
        matrix[i][i]=F(1)
        p_mass={p:F(1)}
        for _ in range(stride):
            q=defaultdict(F)
            for v,mass in p_mass.items():
                steps=[w for w in m.ns(v) if w in body or w in gates]
                for w in steps:
                    if w in body:q[w]+=mass/len(steps)
            p_mass=q
        for w,mass in p_mass.items():
            if w in safe:matrix[i][n]+=mass
            else:matrix[i][index[w]]-=mass
    for j in range(n):
        i=next(i for i in range(j,n) if matrix[i][j])
        matrix[i],matrix[j]=matrix[j],matrix[i]
        val=matrix[j][j]
        matrix[j]=[a/val for a in matrix[j]]
        for i in range(n):
            if i==j:continue
            val=matrix[i][j]
            matrix[i]=[a-val*b for a,b in zip(matrix[i],matrix[j])]
    out={p:F(1) if p in safe else matrix[index[p]][n] for p in body}
    assert all(isinstance(x,F) and 0<=x<=1 for x in out.values())
    return out

def describe(body,gates,safe_for):
    ring={v for p in body for v in m.ns(p)}-set(body)
    entries={p for g in gates for p in m.ns(g) if p in body}
    return {"entries":sorted(entries),"solid_walls":len(ring)-len(gates),"probabilities":{
        str(k):[str(capture_prob(body,gates,safe_for(k),k)[p]) for p in sorted(entries)]
        for k in (1,2,3)}}

def main():
    import pathlib
    data=json.loads((pathlib.Path(__file__).resolve().parent/"result.json").read_text())
    body={(0,y) for y in range(4)}
    gates={(0,-1),(0,4)}
    ring={v for p in body for v in m.ns(p)}-body
    stock=describe(body,gates,lambda _:body-{v for g in gates for v in m.ns(g)})
    results={}
    for n, group in data.items():
        rows=[]
        for row in group["minimum_wall_layouts"]:
            body=set(map(tuple,row["body"]));g,h=map(tuple,row["gates"])
            walls=set(map(tuple,row["walls"]))
            rows.append(describe(body,{g,h},lambda k:safe_starts(body,walls,g,h,k)))
        results[n]=rows
    assert stock["probabilities"]["1"]==["1/2","1/2"]
    print(json.dumps({"reference_4_line_two_guards":stock,"fallback_minimal":results},indent=2))
if __name__=="__main__":main()
