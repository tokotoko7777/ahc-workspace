from pathlib import Path
import sys,json
import numpy as np

workspace=Path(__file__).resolve().parent.parent
sys.path.insert(0,str(workspace/'ahc008_terry_passive_opportunity_atlas'))
from atlas import neighbors,prob,terrain,cells,mask,ALL,B,flood

N=900
NS=[neighbors(i) for i in range(N)]
PAR=np.array([sum(divmod(i,30))%2 for i in range(N)])
DIRECTIONS=np.array([[*a,*([N]*(4-len(a)))]for a in NS])
def quantile(a):
    n=(len(a)-1)//10
    return float(np.partition(a,n)[n])
def complete_map():
    stock=terrain()[3][0]
    catalog={}
    for z in (workspace/'ahc008_terry_small_rooms/catalog.tsv').read_text().splitlines():
        m,i,l,r,*a=map(int,z.split())
        if m!=3:continue
        gates={q:t for q,t in ((l,l-1),(r,r+1)) if q>=0}
        catalog[i]=(set(a),gates)
    assert len(catalog)==75
    return stock,catalog
def certify(stock,catalog,active):
    walls=stock|mask(active.values())
    avail=ALL^walls
    if flood(avail&-avail,avail)!=avail:return None
    target=set()
    for i,(body,all_gates) in catalog.items():
        gates={g:t for g,t in all_gates.items() if active.get(i)!=g}
        assert gates
        mm=mask(gates)
        if mm&walls:return None
        for g,t in all_gates.items():
            if not 0<=t<N or t not in NS[g] or walls&B[t]:return None
        locked=avail&~mm
        actual=flood(B[min(body)],locked)
        if actual!=mask(body) or any(actual&B[t] for t in all_gates.values()):return None
        # The guard side must remain a single connected exterior under each
        # own lock, not just lie outside the body in a separate orphan.
        outside=locked&~actual
        if flood(outside&-outside,outside)!=outside:return None
        if len(body)<=4:target|=body-{v for g in gates for v in NS[g]}
    return sorted(cells(avail)),target
def calc(available,target,stride):
    op=np.zeros(N+1,dtype=bool);op[available]=True
    adj=np.where(op[DIRECTIONS],DIRECTIONS,N)
    zero=np.sum(adj!=N,axis=1)==0
    adj[zero,0]=np.flatnonzero(zero)
    den=np.sum(adj!=N,axis=1)
    value=np.zeros(N+1); which=sorted(target);value[which]=1
    for j in range(25):
        for k in range(stride):
            value[:N]=np.sum(value[adj],axis=1)/den
        value[which]=1
    s=[value[np.array(available)[PAR[available]==p]] for p in (0,1)]
    return (min(map(quantile,s)),min(float(a.mean()) for a in s),float(value[available].mean())), [list(x) for x in zip(map(quantile,s),[float(a.mean()) for a in s])],value[:N]
def main():
    stock,cat=complete_map()
    opts=[(i,g)for i,(body,gs) in sorted(cat.items())if len(body)==3 and len(gs)==2 for g in sorted(gs)]
    assert len(opts)==56
    active={};hist=[]
    b=certify(stock,cat,active)
    assert b is not None
    legal,target=b
    init=calc(legal,target,2)
    nbs=[[q for q in NS[i] if q in set(legal)]or[i] for i in range(N)]
    oracle=prob(nbs,2,target,(25,))[25]
    assert np.max(np.abs(init[2]-oracle)[legal])<1e-14
    def stat(b):
        return {k:{"ordered_min_q10_min_mean_all":list(t[0]),"parity_q10_mean":t[1]} for k in (1,2,3) for t in [calc(*b,k)]}
    init_full=stat(b)
    base=init[0]
    for _ in range(8):
        best=None;tested=0
        for i,g in opts:
            if i in active:continue
            with_cap={**active,i:g}
            new=certify(stock,cat,with_cap)
            if new is None:continue
            tested+=1
            s,par,_=calc(*new,2)
            key=s+(-g,)
            if best is None or key>best[0]:best=(key,i,g,par)
        if best is None or best[0][:3]<=base:break
        key,i,g,par=best;base=key[:3];active[i]=g
        hist.append({"cap_room":i,"gate":g,"opened_eligible_alternatives":tested,
                     "pig_objective":base,"parity":par})
    final=stat(certify(stock,cat,active))
    passes=bool(base[0]>=.25 and base[1]>=.65 and all(init_full[k]["ordered_min_q10_min_mean_all"][2]-final[k]["ordered_min_q10_min_mean_all"][2]<.03 for k in (1,3)))
    print(json.dumps(dict(initial=init_full,chosen=hist,final=final,passes=passes),indent=2))
if __name__=="__main__":main()
