"""Run fresh paired tests and validate geometry and exact rational score."""
import argparse
import csv
from fractions import Fraction as F
import hashlib
from pathlib import Path
import re
import subprocess
import tempfile
import time


def replay(data, output):
    n, m, k = map(int, data[0].split())
    count = n + m
    points = [tuple(map(int, x.split())) for x in data[1:1+count]] + [(0, 5000)]
    p = [[F(z.decode()) for z in x.split()] for x in data[1+count:]]
    assert len(p) == k
    values = [list(map(int, x.split())) for x in output.splitlines()]
    assert len(values) == m + 2
    assignment = values[0]
    assert sorted(assignment) == list(range(n)) and len(values[1]) == 1
    rows = values[2:]
    assert all(r == [-1] or len(r) == 3 and 0 <= r[0] < k for r in rows)
    g = [[] if r == [-1] else r[1:] for r in rows]
    g = [[] for _ in range(n)] + g + values[1:2]
    edges = [(a,b) for a, to in enumerate(g) for b in to]
    assert all(0 <= b < count and (b < n or g[b]) for a,b in edges)

    def orientation(a, b, c):
        return (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])

    def intersect(i, j, k, l):
        a,b,c,d = (points[t] for t in (i,j,k,l))
        box = all(max(min(a[i],b[i]),min(c[i],d[i])) <= min(max(a[i],b[i]),max(c[i],d[i]))
                  for i in range(2))
        return box and orientation(a,b,c)*orientation(a,b,d) <= 0 and orientation(c,d,a)*orientation(c,d,b) <= 0

    for j,(a,b) in enumerate(edges):
        assert all(len({a,b,c,d}) < 4 or not intersect(a,b,c,d) for c,d in edges[:j])
    state = [0]*len(g)
    order=[]
    def dfs(v):
        assert state[v] != 1
        if state[v] == 2:
            return
        state[v] = 1
        for b in g[v]:
            dfs(b)
        state[v] = 2
        order.append(v)
    dfs(count)
    reachable=sum(state[v] == 2 for v in range(n))
    for v in range(count):
        dfs(v)
    correct = F(0)
    for c in range(n):
        prob = [F(0)]*len(g)
        prob[count]=1
        for v in reversed(order):
            if v == count:
                prob[g[v][0]] += prob[v]
            elif v >= n and g[v]:
                chance = p[rows[v-n][0]][c]
                a,b=g[v]
                prob[a] += prob[v]*chance
                prob[b] += prob[v]*(1-chance)
        correct += prob[assignment.index(c)]
    raw = (1-correct/n)*10**9
    return (2*raw.numerator+raw.denominator)//(2*raw.denominator), reachable


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('gen','vis','base','fit','csv'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    gen,vis,base,fit=(getattr(a,k).resolve() for k in ('gen','vis','base','fit'))
    with tempfile.TemporaryDirectory() as d,a.csv.open('w',newline='') as file:
        w = csv.writer(file,lineterminator='\n')
        w.writerow(('split','seed','arm','score','leaves','base','n','m','k','seconds'))
        work=Path(d)
        seeds,ans=work/'seeds.txt',work/'answer.txt'
        for split in ('dev','holdout'):
            ids=[int.from_bytes(hashlib.sha256(f'ahc051/planar-affine-v1/{split}/{i}'.encode()).digest()[:8],
                                'little') for i in range(28)]
            seeds.write_text(''.join(f'{s}\n' for s in ids))
            subprocess.run([gen,seeds],cwd=work,check=True)
            for i,s in enumerate(ids):
                inp=work/'in'/f'{i:04d}.txt'
                data=inp.read_bytes()
                starts=[]
                for label,binary in [('baseline',base),('fit',fit)][::(-1 if i%2 else 1)]:
                    start=time.monotonic()
                    run=subprocess.run([binary],input=data,capture_output=True,check=True,timeout=2)
                    elapsed=time.monotonic()-start
                    score,leaves=replay(data.splitlines(),run.stdout)
                    detail=re.fullmatch(rb'base=(\d+) final=(\d+)\s*',run.stderr)
                    assert detail and int(detail[2])==score and elapsed<2
                    starts.append(int(detail[1]))
                    ans.write_bytes(run.stdout)
                    judge=subprocess.run([vis,inp,ans],cwd=work,capture_output=True,text=True,check=True)
                    assert judge.stdout.strip()==f'Score = {score}',judge.stdout
                    w.writerow((split,s,label,score,leaves,starts[-1],*map(int,data.splitlines()[0].split()),f'{elapsed:.6f}'))
                    file.flush()
                assert starts[0]==starts[1]


if __name__=='__main__':
    main()
