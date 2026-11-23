from pathlib import Path
import sys, json, collections
root=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(root/'ahc008_terry_map3_pig_parity_caps'))
from caps import complete_map,calc,certify
from atlas import mask,cells,neighbors,prob,ALL
import numpy as np

def stats(b):
    a,target=b
    out={}
    for k in (1,2,3):
        p,s,x=calc(a,target,k)
        out[k]={"ordered_min_q10_min_mean_all":list(p),"parity_q10_mean":s}
    return out

def phase(w,rooms):
    old=[(i,a,g)for i,(a,g) in rooms.items()if {u%30 for u in a}=={2,3,4}]
    assert len(old)==15
    old.sort(key=lambda t:min(t[1]))
    assert sorted(len(a) for _,a,g in old)==[3]*14+[4]
    assert all(set(t%30 for t in g)=={1,5}for _,a,g in old)
    nm=dict(rooms)
    centers=[*range(0,15,2),*range(17,30,2)]
    for (i,a,g),r in zip(old,centers):
        row=set(r*30+c for c in (2,3,4))
        if r==14:row.add(15*30+3)
        nm[i]=(row,{r*30+1:r*30,r*30+5:r*30+6})
    slab=mask(r*30+c for r in range(30)for c in (2,3,4))
    assert (w&slab).bit_count()==44
    nwe=w&~slab
    nwe|=mask(r*30+c for r in range(30)if r not in centers for c in (2,3,4)
                 if (r,c)!=(15,3))
    assert w.bit_count()==nwe.bit_count()==233
    assert collections.Counter(len(a)for a,g in rooms.values())==collections.Counter(len(a)for a,g in nm.values())
    return nwe,nm

def main():
    w,old=complete_map()
    ww,new=phase(w,old)
    # Construction not tested. Verify the geometry before starting probabilities.
    pre=certify(w,old,{})
    post=certify(ww,new,{})
    if post is None:
        print(json.dumps({"certified":False}))
        return
    a,t=pre
    op=set(a)
    adj=[[q for q in neighbors(i) if q in op]or[i] for i in range(900)]
    oracle=prob(adj,2,t,(25,))[25]
    assert np.max(np.abs(calc(a,t,2)[2]-oracle)[a])<1e-14
    x,y=stats(pre),stats(post)
    values=lambda k: y[k]["ordered_min_q10_min_mean_all"]
    gain=(values(2)[0]>=.25 and values(2)[1]>=.65 and all(
        x[k]["ordered_min_q10_min_mean_all"][2]-values(k)[2]<.03 for k in (1,3)))
    print(json.dumps(dict(certified=True,before=x,after=y,passes=gain,
                          rooms=len(new),planned_x=ww.bit_count()),indent=2))
if __name__=="__main__":main()
