"""Exact local placement of frozen six-cell dual-gate shape classes."""
import sys,json
from pathlib import Path
from collections import Counter
from hashlib import sha256
D=Path(__file__).resolve().parent;R=D.parent
sys.path.insert(0,str(R/'ahc008_terry_hunt_wall_audit'))
from analyze import terrain,cells,mask
from stochastic_lock_audit import ALL,B,flood,wave
F=R/'ahc008_terry_one_guard_dual_fallback/result.json'
X=R/'ahc008_terry_small_rooms/catalog.tsv'
assert sha256(X.read_bytes()).hexdigest()=='2d697e1b2cbf0cd9d754d38a18059e2fbcd484b7440868da8016f8310132ba06'
forms=json.loads(F.read_text())['6']['minimum_wall_layouts']
assert len(forms)==17 and all(len(s['walls'])==8 and len(s['body'])==6 for s in forms)
k=1; w,_,=terrain()[k]
rooms=[]
for z in X.read_text().splitlines():
    m,i,l,r,*p=map(int,z.split())
    if m==k:
        rooms.append((i,mask(p),mask(q for q in (l,r) if q>=0),
          mask(q for q,ok in ((l-1,l>=0),(r+1,r>=0)) if ok)))
assert len(rooms)==76
aisle=mask(r*30+c for r in range(30) for c in (4,14,24))
assert not aisle&w
def connected(w):
    return bool((flood(aisle&-aisle,ALL^w)&aisle)==aisle)
def normalize(s,k):
    parts=[]
    for kind in ('body','gates','stances','walls'):
        x=[]
        for i,j in s[kind]:
            if k&4:i,j=j,i
            x.append(((-i if k&1 else i),(-j if k&2 else j)))
        parts.append(x)
    y=min(i for i,j in parts[0]);x=min(j for i,j in parts[0])
    return tuple(tuple(sorted((r-y,c-x) for r,c in a)) for a in parts)
shapes=sorted({normalize(s,k)for s in forms for k in range(8)})
counts=Counter(); rows=[]
for ident,body,doors,poses in rooms:
    if body.bit_count()!=9 or doors.bit_count()!=2:continue
    protected=[(b,g,s) for i,b,g,s in rooms if i!=ident]
    old_region=body|doors
    old_open=flood(aisle&-aisle,ALL^w)
    old_score=flood(aisle&-aisle,ALL^(w|doors)).bit_count()
    assert old_score+11==old_open.bit_count() and old_open==(ALL^w)
    blocked=0
    for b,g,s in protected:blocked|=b|g|s
    found=None;c=Counter()
    for shape in shapes:
        # At least one geometry role must touch the old region, even
        # if the entire new body lies just outside.
        indices=set(q for part in shape for q in part)
        origins={(r-y,c-x)for q in cells(old_region)for r,c in [divmod(q,30)]for y,x in indices}
        for a,d in sorted(origins):
            moved=tuple(tuple((r+a,c+d)for r,c in part)for part in shape)
            if any(not(0<=r<30 and 0<=c<30) for part in moved[:3] for r,c in part):continue
            b,g,s,x=(mask(r*30+c for r,c in part if 0<=r<30 and 0<=c<30)for part in moved)
            c['near_inboard']+=1
            if (b|g|s)&w:continue
            c['stock_x_preserved']+=1
            extra=x&~w
            if extra&~old_region:continue
            c['local_new_x']+=1
            if extra.bit_count()>2:continue
            c['up_to_two_new_x']+=1
            if (b|g|extra)&blocked or (b|g|extra)&aisle:continue
            c['other_roles_preserved']+=1
            nw=w|extra
            if flood(b&-b,ALL^(nw|g))!=b:continue
            open2=flood(aisle&-aisle,ALL^nw)
            close2=flood(aisle&-aisle,ALL^(nw|g))
            if not (open2&s and close2&s and open2&aisle==aisle and close2&aisle==aisle):continue
            if old_open&~(open2|extra):continue
            if any(flood(u&-u,ALL^(nw|v))!=u or t&~flood(aisle&-aisle,ALL^(nw|v))
                 for u,v,t in protected):continue
            gain=close2.bit_count()-old_score
            if gain<1:continue
            c['verified']+=1
            z=(extra.bit_count(),-gain,a,d,shape)
            if found is None or z<found:found=z
    counts.update(c)
    rows.append(dict(room=ident,counts=c,best=found))
assert len(rows)==14
print(json.dumps(dict(oriented_shapes=len(shapes),old_room_count=len(rows),counts=counts,
           shapes_hash=sha256(F.read_bytes()).hexdigest(),rows=rows),indent=2))
