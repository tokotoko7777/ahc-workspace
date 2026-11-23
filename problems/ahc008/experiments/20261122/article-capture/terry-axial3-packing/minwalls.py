from check import norm,shift,compatibility,adj
import json
H=30
A=norm(); N=len(A)
def h(b):
 return 1+max(r for r,c in b[0])
pos=[(r,i)for r in range(H)for i,a in enumerate(A)if r+h(a)<=H]
walls={(r,i):{(y,c)for y,c in shift(A[i][2],r)if 0<=y<H}for r,i in pos}
trans={}
for a in pos:
 x,i=a
 trans[a]=[]
 for b in pos:
  y,j=b
  if y<x+2 or not compatibility(A[i],A[j],y-x):continue
  trans[a].append((b,len(walls[b]-walls[a])))
assert all(max(r for r,c in a[2])-min(r for r,c in a[2])<=3 for a in A)
def flood(p,S):
 got={p};t=got
 while t:
  t=(set.union(*(adj(x)for x in t))&S)-got
  got|=t
 return got
def verify(path):
 grid={(r,c) for r in range(H)for c in range(-2,5)}
 ww=set.union(*(walls[p]for p in path)); op=grid-ww
 if flood((0,-2),op)!=op:return False
 for r,i in path:
  b,g=shift(A[i][0],r),shift(A[i][1],r)
  z=op-g
  if flood(min(b),z)!=b or flood((0,-2),z)!=(z-b):return False
 return True
def main():
 dp={(int(A[i][3]),p):(len(walls[p]),(p,))for p in pos for r,i in [p]}
 profile={};witness={}
 for k in range(1,16):
  best={}
  for (q,p),v in dp.items():
   if q not in best or v<best[q]:best[q]=v
  profile[k]={q:c for q,(c,_)in sorted(best.items())}
  if k>=12:
   counts=[q for q,(c,_) in best.items() if c<=44]
   if counts:
    q=max(counts); c,ps=best[q]
    assert c==len(set.union(*(walls[p]for p in ps)))
    witness[k]={"max_both_at_44":q,"min_wall":c,"verified_two_rail":verify(ps),
                 "placements":[dict(row=r,body=sorted(A[i][0]),doors=sorted(A[i][1]))for r,i in ps]}
  nxt={}
  for (q,p),(c,path) in dp.items():
   for new,extra in trans[p]:
    key=(q+int(A[new[1]][3]),new)
    if key not in nxt or c+extra<nxt[key][0]:
     nxt[key]=(c+extra,path+(new,))
  dp=nxt
 print(json.dumps(dict(min_walls=profile,budget_witness=witness),separators=(',',':')))
if __name__=='__main__':main()
