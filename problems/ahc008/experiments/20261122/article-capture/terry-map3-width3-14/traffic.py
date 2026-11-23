from pathlib import Path
import sys,json
import numpy as np
root=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(root/'ahc008_terry_map3_pig_parity_caps'))
from caps import complete_map,calc,certify
from atlas import neighbors,prob,mask
def phase(w,old):
    witness=json.loads((root/'ahc008_terry_axial3_packing/walls.json').read_text())['budget_witness']['14']
    assert witness['max_both_at_44']==7 and witness['min_wall']==44
    cut={i for i,(b,g) in old.items()if {v%30 for v in g}=={1,5}}
    edit=mask(r*30+c for r in range(30) for c in range(1,6))
    assert len(cut)==15 and (w&edit).bit_count()==44
    template=[];new={i:role for i,role in old.items()if i not in cut};ww=w&~edit
    for i,z in zip(sorted(cut),witness['placements'],strict=False):
        r=z['row']
        body={(r+a)*30+c+2 for a,c in z['body']}
        g={(r+a)*30+c+2 for a,c in z['doors']}
        st={p:p-1 if p%30==1 else p+1 for p in g}
        assert {p%30 for p in g}=={1,5}
        s={q for p in body for q in neighbors(p)}-body-g
        added=(body,g,s)
        assert not any(b&(body|g|s) or gates&(body|g|s) or shell&(body|g)
                       for b,gates,shell in template)
        template.append(added);ww|=mask(s)
        new[i]=(body,st)
    assert (ww&edit).bit_count()==44 and ww.bit_count()==233 and len(new)==74
    return ww,new

def stats(valid):
    return {k:dict(objective=list(a),by_parity=b)for k in (1,2,3) for a,b,_ in [calc(*valid,k)]}
def main():
    w,old=complete_map()
    ww,new=phase(w,old)
    pre,post=certify(w,old,{}),certify(ww,new,{})
    assert pre and post
    for op,g in (pre,post):
        s=set(op);adj=[[q for q in neighbors(i) if q in s]or [i]for i in range(900)]
        assert np.max(np.abs(prob(adj,2,g,(25,))[25]-calc(op,g,2)[2])[op])<1e-14
    a,b=stats(pre),stats(post); key=lambda k:b[k]['objective']
    passed=key(2)[0]>=.25 and key(2)[1]>=.65 and all(a[k]['objective'][2]-key(k)[2]<.03 for k in (1,3))
    print(json.dumps(dict(rooms=len(new),wall_count=ww.bit_count(),stock=a,witness=b,passes=passed),indent=2))
if __name__=='__main__':main()
