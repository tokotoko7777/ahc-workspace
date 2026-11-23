"""Independent physical Map3 overlay of one fixed k14 canonical witness."""
import sys,json
from pathlib import Path
from hashlib import sha256
D=Path(__file__).resolve().parent; R=D.parent
P=R/'ahc008_terry_map3_staggered_left'
F=R/'ahc008_terry_axial3_packing/walls.json'
def sha(a):return sha256(a.read_bytes()).hexdigest()
assert sha(F)=='93a601cb92419a8003dd578b275a05990287bfa0a9b5a25d013ab6beb9137c08'
assert sha(P/'check.py')=='e289db591a145af4c15bfd4166c390b2a145a9b25080038257078119612487b5'
sys.path.insert(0,str(P))
from check import parse,adj,flood,BOARD,AISLE,ROOT
spec=json.loads(F.read_text())['budget_witness']['14']
assert spec['min_wall']==44 and spec['max_both_at_44']==7 and spec['verified_two_rail']
old,rooms=parse()
oldbank=sorted([z for z in rooms if {c for r,c in z[2]}=={1,5}],
               key=lambda q:min(q[2]))
keep=[z for z in rooms if z not in oldbank]
assert len(oldbank)==15 and len(keep)==60
slab={(r,c)for r in range(30)for c in range(1,6)}
new=[]
for oldrow,s in zip(oldbank,spec['placements']):
    body={(s['row']+r,c+2)for r,c in s['body']}
    gates={(s['row']+r,c+2)for r,c in s['doors']}
    stance={(r,0 if c==1 else 6) for r,c in gates}
    assert (body|gates)<=slab and stance<=AISLE
    assert {c for r,c in gates}=={1,5}
    new.append((oldrow[0],body,gates,stance))
assert len(new)==len(spec['placements'])==14
shell=set()
for _,b,g,s in new:
    shell|=set().union(*(adj(q) for q in b))-b-g
assert shell<=slab and len(shell)==44
walls=(old-slab)|shell
assert len(walls)==233 and len(old)==233 and (old^walls)<=slab
allrooms=keep+new
assert len(allrooms)==74 and len({i for i,b,g,s in allrooms})==74
floor=BOARD-walls; used=set(); stances=set(); allg=set()
for _,b,g,s in allrooms:
    assert not (b|g)&(used|walls|stances|s) and not s&(used|walls)
    used|=b|g;stances|=s;allg|=g
    assert all(len(adj(q)&b)==len(adj(q)&s)==1 for q in g)
assert AISLE<=floor
assert flood(ROOT,floor)==floor
for _,b,g,s in allrooms:
    a=flood(ROOT,floor-g)
    assert a==floor-g-b and (s|AISLE|stances)<=a
    assert flood(min(b),floor-g)==b
    assert flood(min(b),floor-allg)==b
    assert flood(min(b),b)==b
core=floor-used
seen=set();regions=[]
for p in sorted(AISLE):
    if p in seen:continue
    r=flood(p,core);regions.append(len(r));seen|=r
assert seen==core
safe=[b-set().union(*(adj(q)for q in g))for _,b,g,s in new]
colors=[{sum(x)%2 for x in a}for a in safe]
assert sum(e=={0,1}for e in colors)==7
print(json.dumps(dict(rooms=74,preserved_stock_roles=60,wall_count=233,
    slab_walls=44,all_open_floor=len(floor),all_closed_exterior_areas=regions,
    own_lock_footprints_and_service=74,all_lock_exact_bodies=74,both_color_left_bank=7,
    masks=dict(wall_rows=[sum(1<<c for r,c in walls if r==y)for y in range(30)],
               new_left=[dict(id=i,body=sorted(b),doors=sorted(g),stances=sorted(s))for i,b,g,s in new])),
    indent=2))
