"""Fixed Map3 parity-seam geometry only, no game inputs."""
from pathlib import Path
import hashlib, re, json
from collections import Counter,deque
D=Path(__file__).resolve().parent; R=D.parent
T=R/'ahc008_terry_exact_maps/terrain_data.inc'
C=R/'ahc008_terry_small_rooms/catalog.tsv'
N=30
BOARD=set((y,x)for y in range(30)for x in range(30))
AISLE={(y,x) for y in range(30) for x in (0,6,13,19,26)}
ROOT=(0,0)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(T)=='8d1f703d67b9d16fb0ba6d3b20cd2bb4a2cf20aea33e4a7351d45fd129e256cf'
assert sha(C)=='2d697e1b2cbf0cd9d754d38a18059e2fbcd484b7440868da8016f8310132ba06'
def adj(p):
    r,c=p
    return {(r-1,c),(r+1,c),(r,c-1),(r,c+1)}&BOARD
def flood(start,space):
    assert start in space
    out={start};q=deque([start])
    while q:
        x=adj(q.popleft())&(space-out)
        out|=x;q.extend(x)
    return out
def parse():
    fields=re.findall(r'\{((?:0x[0-9a-f]+u,){4}0x[0-9a-f]+u)\}',
                      re.split(r'// map \d\s*\n',T.read_text())[3])
    rows=[[int(v[:-1],16) for v in s.split(',')] for s in fields]
    assert len(rows)==5
    def entry(r):return rows[0 if r==0 else 1 if r%2==0 else 3 if r%4==3 else 2]
    walls={(r,c)for r,c in BOARD if entry(r)[0]>>c&1}
    doors={(r,c)for r,c in BOARD if any(k>>c&1 for k in entry(r)[1:])}
    rs=[]
    for line in C.read_text().splitlines():
        m,i,l,r,*base=map(int,line.split())
        if m!=3:continue
        bp={divmod(p,N)for p in base}
        dp={divmod(p,N)for p in (l,r) if p>=0}
        sp={divmod(p,N)for p,b in ((l-1,l>=0),(r+1,r>=0)) if b}
        rs.append((i,bp,dp,sp))
    assert set().union(*(x[2]for x in rs))==doors
    return walls,rs
def checks(w,rs):
    assert len(w)==233 and len(rs)==75 and not w&AISLE
    assert len({a for a,*_ in rs})==75
    used=set(); poses=set()
    for i,b,g,s in rs:
        assert not (b|g)&(used|w|s) and not (b|g|w)&poses
        used|=b|g;poses|=s
        assert all(len(adj(q)&b)==len(adj(q)&s)==1 for q in g)
        assert s<=AISLE
    pp=BOARD-w
    allg=set().union(*(g for i,b,g,s in rs))
    assert flood(ROOT,pp)==pp
    for i,b,g,s in rs:
        assert flood(min(b),pp-g)==b
        assert flood(min(b),pp-allg)==b
        assert flood(ROOT,pp-g)==pp-g-b
        assert s<=flood(ROOT,pp-g) and AISLE<=flood(ROOT,pp-g)
    core=pp-used
    # All-closed exterior may have multiple lane components; never
    # count a floor component divorced from all of those real aisles.
    seen=set();comp=0
    for q in sorted(AISLE):
        if q not in seen:
            seen|=flood(q,core);comp+=1
    assert seen==core
    return dict(exterior_all_closed_components=comp,physical_open_floor=len(pp),
                exact_own_locks=75,exact_all_gates_footprints=75)
def pack(w,r):
    def pack1(v):return [sum(1<<c for y,c in v if y==i) for i in range(30)]
    return dict(X=pack1(w),rooms=[dict(stock_room_id=i,body=sorted(b),doors=sorted(g),stances=sorted(s))
                                  for i,b,g,s in r])
def main():
    old,rs=parse()
    a=checks(old,rs)
    left=sorted((v for v in rs if min(v[2])[1]==1 and max(v[2])[1]==5
                                  and len(v[2])==2),key=lambda v:min(v[2]))
    assert len(left)==15 and [min(v[2])[0]for v in left]==list(range(1,30,2))
    assert Counter(len(v[1])for v in left)=={3:14,4:1}
    r2=list(range(0,15,2))+list(range(17,30,2))
    keep=[v for v in rs if v not in left]
    newleft=[]
    for k,r in enumerate(r2):
        b={(r,c)for c in (2,3,4)}
        if r==14:b.add((15,3))
        newleft.append((left[k][0],b,{(r,1),(r,5)},{(r,0),(r,6)}))
    wr={(r,c)for r in range(30)for c in (2,3,4)
        if r not in r2 and (r,c)!=(15,3)}
    wa={q for q in old if q[1] in (2,3,4)}
    assert len(wr)==len(wa)==44
    ww=(old-wa)|wr
    r=sorted(keep+newleft)
    v=checks(ww,r)
    for z in keep:assert z in r
    def safe_by_parity(rr):
        counts=Counter()
        for i,b,g,s in rr:
            for r,c in b-set().union(*(adj(q)for q in g)):
                counts[(r+c)%2]+=1
        return counts
    ans=dict(stock=a,staggered=v,old_X=len(old),new_X=len(ww),removed_X=sorted(old-ww),
        inserted_X=sorted(ww-old),preserved_other_bank_rooms=len(keep),
        unchanged_straight_centers=[x for x in r2 if x>16],
        left_old_safe_cells=safe_by_parity(left),left_new_safe_cells=safe_by_parity(newleft),
        masks=pack(ww,r))
    print(json.dumps(ans,indent=2))
if __name__=='__main__':main()
