"""Recover unique-walk contigs and measure on independently simulated AHC004 boards."""
import random, collections, statistics, sys
from time import perf_counter

def unitigs(reads,k):
    pred=collections.defaultdict(set)
    succ=collections.defaultdict(set)
    for s in set(reads):
        for i in range(len(s)-k):
            a=s[i:i+k]; b=s[i+1:i+k+1]
            succ[a].add(b);pred[b].add(a)
    seen=set()
    contigs=set()
    def walk(a,b):
        st=a;s=a
        while (a,b) not in seen:
            seen.add((a,b));s+=b[-1]
            if len(s)==20+k:
                if b==st:s=s[:20]
                else:return
                break
            if len(pred[b]) != 1 or len(succ[b]) != 1:break
            a,b=b,next(iter(succ[b]))
        if len(s)<=20:contigs.add(s)
    for a,bs in sorted(succ.items()):
        if len(pred[a])!=1 or len(bs)!=1:
            for b in sorted(bs):walk(a,b)
    for a,bs in sorted(succ.items()):
        for b in sorted(bs):
            if (a,b) not in seen:walk(a,b)
    return contigs

def make_case(L,M,seed):
    rng=random.Random(seed)
    b=[''.join(rng.choices('ABCDEFGH',k=20)) for _ in range(20)]
    lines=b+list(map(''.join,zip(*b)))
    rr=[]
    for _ in range(M):
        s=rng.choice(lines); st=rng.randrange(20); n=rng.randrange(L-2,L+3)
        rr.append((s+s)[st:st+n])
    return [s+s for s in lines],rr

def evaluate(lines,pool,mn):
    positions=[0]*40
    nc=nt=0; len_true=[]
    for s in pool:
        if len(s)<mn:continue
        nc+=1; hit=False
        for i,line in enumerate(lines):
            q=line.find(s)
            while 0<=q<20:
                positions[i]|=sum(1<<((q+j)%20) for j in range(len(s)))
                q=line.find(s,q+1);hit=True
        if hit:nt+=1;len_true.append(len(s))
    return nc,nt,sum(x.bit_count() for x in positions),sum(x.bit_count()>=10 for x in positions),sum(x.bit_count()>=15 for x in positions),max(len_true or [0])

def main():
    t=perf_counter()
    for L in range(5,9):
        for M in [450,650,800]:
            res=collections.defaultdict(list)
            for i in range(36):
                lines,r=make_case(L,M,1_970_000_007+1000*L+i)
                u3,u4,u5=[unitigs(r,k) for k in [3,4,5]]
                for name,cont,minlen in [('reads',r,8),('u5',u5,8),('u4',u4,8),('u4+5',u4|u5,8),('u3',u3,8),('u4+5@12',u4|u5,12)]:
                    res[name].append(evaluate(lines,cont,minlen))
            for n,arr in res.items():
                a,b,c,d,e,f=map(statistics.mean,zip(*arr))
                print(f'{L=} {M=} {n:8} cand={a:5.1f} precision={b/a if a else 0:.3f} positions={c:5.1f}/800 lines10={d:4.1f}/40 lines15={e:4.1f}/40 longest={f:4.1f}')
    print('sec',perf_counter()-t)
if __name__=='__main__':main()
