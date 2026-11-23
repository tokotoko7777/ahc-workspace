from itertools import combinations
import json

def adj(p):
 r,c=p
 return {(r+1,c),(r-1,c),(r,c+1),(r,c-1)}

def connected(S):
 U={min(S)}
 while True:
  T=U|(set.union(*(adj(p)for p in U))&S)
  if T==U:return S==T
  U=T

def shell(b):
 return set.union(*(adj(p)for p in b))-b
def norm():
 out=[]
 for n in (3,4):
  for b0 in combinations([(r,c) for r in range(2)for c in range(3)],n):
   b=set(b0)
   if min(r for r,c in b)!=0 or {c for r,c in b}!={0,1,2} or not connected(b):continue
   s=shell(b)
   for l in sorted(p for p in s if p[1]==-1):
    for r in sorted(p for p in s if p[1]==3):
     safe=b-adj(l)-adj(r)
     out.append((b,{l,r},s-{l,r},len({(i+j)%2 for i,j in safe})==2))
 return out

def shift(S,p):
 return {(r+p,c)for r,c in S}

def compatibility(a,b,d):
 ap,ag,aw,_=a
 bp,bg,bw,_=b
 bs,gs,ws=(shift(s,d)for s in (bp,bg,bw))
 return not (ap&(bs|gs|ws) or ag&(bs|gs|ws) or aw&(bs|gs)
             or ap&set.union(*(adj(x)for x in bs)))

def main():
 A=norm();k=len(A)
 assert all(not compatibility(a,b,d) for a in A for b in A for d in (0,1))
 gaps=[[next(d for d in range(2,5) if compatibility(a,b,d)) for b in A]for a in A]
 assert all(d in (2,3) for row in gaps for d in row)
 # Endpoint state's value = min last-pose row and witness ID sequence.
 dp={(int(a[3]),i):(0,[(i,0)]) for i,a in enumerate(A)}
 report={}; witnesses={}
 for rooms in range(1,16):
  win={}
  for (q,i),(s,path) in dp.items():
   v=(s+1+max(r for r,_ in A[i][0]),path)
   if q not in win or v<win[q]:win[q]=v
  report[rooms]={q:h for q,(h,_)in sorted(win.items())}
  if rooms>=12:
   good=[q for q,(h,_)in win.items()if h<=30]
   q=max(good);h,path=win[q]
   assert all(compatibility(A[i],A[j],y-x)for t,(i,x)in enumerate(path)for j,y in path[t+1:])
   witnesses[rooms]=dict(max_both=q,min_height=h,placements=[
       {"row":s,"body":sorted(b),"doors":sorted(g),"both":f}
       for (i,s) in path for (b,g,w,f) in [A[i]]])
  nxt={}
  for (q,i),(s,path) in dp.items():
   for j,b in enumerate(A):
    y=s+gaps[i][j];key=(q+int(b[3]),j);v=(y,path+[(j,y)])
    if key not in nxt or v<nxt[key]:nxt[key]=v
  dp=nxt
 # All-safe best, critical extremes.
 ans=dict(templates=len(A),body_gate_both=sum(int(a[3])for a in A),height=report,
          max_all_both=max(k for k,r in report.items()if r[k]<=30),at_30=witnesses)
 print(json.dumps(ans,separators=(',',':')))
if __name__=='__main__':main()
