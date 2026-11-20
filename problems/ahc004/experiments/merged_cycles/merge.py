from partial import *
def make_patterns(reads,k=6):
    S=sorted({x for x in unitigs(reads,4)|unitigs(reads,5)|set(reads) if len(x)>=k}, key=lambda x:(-len(x),x))
    N=len(S)
    idx=collections.defaultdict(list)
    # Natural contiguous source segments; no unobserved substring across an end.
    for i,s in enumerate(S):
        for t in range(len(s)-k+1):idx[s[t:t+k]].append((i,t))
    edge=collections.Counter()
    for group in idx.values():
        if len(group)>32:continue
        for j,(a,pa) in enumerate(group):
            for b,pb in group[j+1:]:
                if a!=b:edge[a,b,(pa-pb)%20]+=1
    parent=list(range(N)); sh=[0]*N
    P=[list(x.ljust(20,'.')) for x in S]
    def root(x):
        d=0
        while x!=parent[x]:d+=sh[x];x=parent[x]
        return x,d%20
    for (a,b,d),_ in sorted(edge.items(),key=lambda it:(-it[1],it[0])):
        a,u=root(a);b,v=root(b)
        if a==b:continue
        d=(d+u-v)%20
        slots=[(i+d)%20 for i in range(20)]
        if any(P[a][j]!='.' and l!='.' and P[a][j]!=l for j,l in zip(slots,P[b])):continue
        for j,l in zip(slots,P[b]):
            if l!='.':P[a][j]=l
        parent[b]=a;sh[b]=d
    strings=set()
    for i in range(N):
        if parent[i]==i:
            s=''.join(P[i]);strings.add(min(s[z:]+s[:z] for z in range(20)))
    return strings

def evalpatterns(lines,patterns,n):
    pos=[0]*40;c=h=0
    for s in patterns:
        ix=[(i,x) for i,x in enumerate(s) if x!='.']
        if len(ix)<n:continue
        c+=1; ok=False
        # Align only an observed letter as fast filter, then verify all 20 slots.
        a,v=ix[len(ix)//2]
        for i,ls in enumerate(lines):
            st=ls.find(v)
            while 0<=st<20:
                d=(st-a)%20
                if all(ls[d+j]==x for j,x in ix):
                    ok=True
                    pos[i]|=sum(1<<((d+j)%20) for j,_ in ix)
                st=ls.find(v,st+1)
        h+=ok
    return c,h,sum(x.bit_count() for x in pos),sum(x.bit_count()>=15 for x in pos),sum(x.bit_count()==20 for x in pos)

if __name__=='__main__':
    t=perf_counter()
    for L in range(5,9):
        for M in [450,650,800]:
            rs=collections.defaultdict(list)
            for i in range(24):
                lines,r=make_case(L,M,2_110_005_037+1000*L+i)
                for k in (5,6,7):
                    p=make_patterns(r,k)
                    for n in (8,12,16,20): rs[k,n].append(evalpatterns(lines,p,n))
            for (k,n),arr in rs.items():
                a,b,c,d,e=map(statistics.mean,zip(*arr))
                if M==650 or n in (12,20): print(f'{L=} {M=} {k=} {n=} num={a:.1f} true={b/a if a else 0:.3f} positions={c:.0f} lines15={d:.1f} lines20={e:.1f}',flush=True)
    print('sec',perf_counter()-t)
