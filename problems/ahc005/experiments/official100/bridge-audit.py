import sys
from pathlib import Path
from collections import Counter
root=Path(sys.argv[1]); sys.setrecursionlimit(20000)
D={'U':(-1,0),'D':(1,0),'L':(0,-1),'R':(0,1)}
def add(x,d):return x[0]+d[0],x[1]+d[1]
def one(p):
 n,r,c,*board=p.read_text().split(); n,r,c=map(int,(n,r,c)); start=(r,c)
 G={(i,j):int(x) for i,row in enumerate(board) for j,x in enumerate(row) if x!='#'}
 adj={x:[y for d in D.values() if (y:=add(x,d)) in G] for x in G}
 groups=[]; members={}
 for d in ((0,1),(1,0)):
  run={}
  for x in G:
   if add(x,(-d[0],-d[1])) in G:continue
   L=[]; y=x
   while y in G:L.append(y);y=add(y,d)
   g=len(groups);groups.append(L)
   for v in L:run[v]=g
  members[d]=run
 a,b=members.values()
 mandatory={a[x] for x in G if len(groups[b[x]])==1 and len(groups[a[x]])>=2}|{b[x] for x in G if len(groups[a[x]])==1 and len(groups[b[x]])>=2}
 t=[]; tin={}; bridges=[]
 def dfs(u,p):
  l=tin[u]=len(t);t.append(u)
  for v in adj[u]:
   if v==p:continue
   if v in tin:l=min(l,tin[v]);continue
   k=dfs(v,u)
   if k>tin[u]:bridges.append((u,v,tin[v],len(t)))
   l=min(k,l)
  return l
 dfs(start,None);assert len(t)==len(G)
 route=(root/'baseline100'/'baseline'/p.name).read_text().strip()
 counts=Counter(); x=start
 for d in route:y=add(x,D[d]);assert y in G;counts[x,y]+=1;x=y
 assert x==start
 q=Counter()
 for u,v,l,r in bridges:
  S=set(t[l:r]); g=(a if u[0]==v[0] else b)[u]; assert g==(a if u[0]==v[0] else b)[v]
  whole=any(set(groups[vv])<=S for x in S for vv in (a[x],b[x]) if vv in mandatory)
  required=any(x not in groups[g] for x in S); assert not whole or required
  k=counts[u,v]; assert counts[v,u]==k; assert not required or k>0
  cat='re' if required else 'op'
  q[cat]+=1; q['whole']+=whole; q['exact_extra']+=required and not whole
  q[cat+'_entered']+=k>0;q[cat+'_entries']+=k;q[cat+'_repeat']+=k>1
  q['op_savings']+= (G[u]+G[v])*k if not required else 0
  q['re_excess_cost']+=(G[u]+G[v])*max(0,k-1) if required else 0
  q['re_excess_entries']+=max(0,k-1) if required else 0
 return q
A=sorted((root/'in').glob('*.txt'));assert len(A)==100
R=[one(p) for p in A]
for k in sorted(set().union(*(q.keys() for q in R))):print(k,sum(q[k] for q in R),sum(q[k]>0 for q in R),max(q[k] for q in R))
