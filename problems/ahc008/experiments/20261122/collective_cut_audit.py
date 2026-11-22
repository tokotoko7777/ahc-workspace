"""Compare optimistic collective, orthogonal clean-region cuts on actual traces."""
import argparse,csv,re,sys,tempfile
from itertools import combinations
from collections import defaultdict
from pathlib import Path
from collections import deque
from guarded_entry_trace import neigh,observed

LAYOUTS=((2,3),(3,3),(4,4))
TIMES=range(40,211,10)
N={(x,y):neigh((x,y)) for x in range(30) for y in range(30)}
BOARD=set(N)

def layout(nx,ny):
    region={}
    def bounds(n):
        return [(int(30*i/n)+(i>0),29 if i==n-1 else int(30*(i+1)/n)-1) for i in range(n)]
    xs,ys=bounds(nx),bounds(ny)
    for i,(a,b) in enumerate(xs):
        for j,(c,d) in enumerate(ys):
            for x in range(a,b+1):
                for y in range(c,d+1):region[x,y]=i*ny+j
    corners={}
    for p in BOARD-region.keys():
        ss={region[q] for q in N[p] if q in region}
        if len(ss)>=2: corners[p]=ss
    tile_adjacency=[{k for k in range(nx*ny) if
                      abs(k//ny-j//ny)+abs(k%ny-j%ny)==1} for j in range(nx*ny)]
    cells=[{p for p,v in region.items() if v==t} for t in range(nx*ny)]
    return region, corners, tile_adjacency,cells

def parts(points, adjacent):
    left=set(points)
    while left:
        a=min(left)
        q=[a]
        left.remove(a)
        for p in q:
            for t in adjacent[p]:
                if t in left:
                    left.remove(t);q.append(t)
        yield frozenset(q)

def short(start,n,wall):
    q={start}
    seen=set(q)
    for _ in range(n):
        q={b for a in q for b in N[a] if b not in seen and b not in wall}
        seen.update(q)
    return seen

def can_staff(wants,people,n,wall,domain):
    # Optimistic: the same number of preparatory walking steps for each human.
    # Exact assignment of at most one final gate build per human; ignores post-walk pet motion.
    if len(wants)>len(people):return False
    accessible=[short(h,n,wall) for h in people]
    if any(not(a&domain) for a in accessible):return False
    options=[{i for i,a in enumerate(accessible) if any(q in domain and q in a for q in N[w])}
             for w in wants]
    options.sort(key=len)
    def dfs(j,used):
        if j==len(options):return True
        return any(dfs(j+1,used|{i}) for i in options[j]-used)
    return dfs(0,set())

def analyze(shape,history,guard_audit=False):
    region,portals,tile_adjacency,cells=shape
    best=[0]*6
    guard_groups=defaultdict(dict)
    for t in TIMES:
        ps,hs,walls=history[t]
        q=set(ps)
        dirty=set()
        for p in q:
            if p in region:dirty.add(region[p])
            if p in portals:dirty.update(portals[p])
        clean=set(range(len(cells)))-dirty
        forbidden=q|set(hs)|{u for p in q for u in N[p]}
        for group in parts(clean,tile_adjacency):
            best[0]=max(best[0],sum(len(cells[j]) for j in group))
            variant={group}|{frozenset([i]) for i in group}
            if len(group)>2:
                variant.update(group-{i} for i in group)
            for union in variant:
                domain=set().union(*(cells[j] for j in union))
                domain.update(p for p,g in portals.items() if g<=union)
                domain-=walls
                for piece in parts(domain,N):
                    a=len(piece)
                    if a<45 or q&piece:continue
                    best[1]=max(best[1],a)
                    wants={q for p in piece for q in N[p] if q not in piece and q not in walls}
                    if len(wants)<=12:best[2]=max(best[2],a)
                    if len(wants)<=8:best[3]=max(best[3],a)
                    if not wants or wants&forbidden or len(wants)>len(hs):continue
                    if guard_audit and a>=150 and len(wants)>=2:
                        gate_options=[(w,[u for u in N[w] if u in piece]) for w in sorted(wants)]
                        key=frozenset(wants)
                        for (_,aa),(_,bb) in combinations(gate_options,2):
                            for u in aa:
                                for v in bb:
                                    if u==v:continue
                                    pair=tuple(sorted((u,v)))
                                    bucket=guard_groups[pair]
                                    old=bucket.get(key)
                                    if old is None or old[0]<a:
                                        bucket[key]=(a,hs,walls,piece)
                    if can_staff(wants,hs,2,walls,piece):
                        best[4]=max(best[4],a)
                        if can_staff(wants,hs,0,walls,piece):best[5]=max(best[5],a)
    if guard_audit:
        coverage=max((len(x) for x in guard_groups.values()),default=0)
        areas=[0,0]
        # Oracle pair placement: pick top repeated stances using all samples.
        for pair,bucket in sorted(guard_groups.items(),key=lambda x:-len(x[1]))[:10]:
            for demands,(a,hs,walls,piece) in bucket.items():
                if a<=areas[0] and (len(bucket)<2 or a<=areas[1]):continue
                for i,j in combinations(range(len(hs)),2):
                    assigned=tuple(h for k,h in enumerate(hs) if k not in (i,j))+pair
                    if can_staff(demands,assigned,2,walls,piece):
                        areas[0]=max(areas[0],a)
                        if len(bucket)>=2:areas[1]=max(areas[1],a)
                        break
        return best+[coverage]+areas
    return best

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("harness",type=Path)
    p.add_argument("tester",type=Path)
    p.add_argument("solver",type=Path)
    p.add_argument("cases",type=Path)
    p.add_argument("--guard-audit",action="store_true")
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester,solver=a.tester.resolve(),a.solver.resolve()
    harness=a.harness.resolve()/"benchmark_cases.py"
    writer=csv.writer(sys.stdout,lineterminator="\n")
    writer.writerow(["case","layout","max_ideal_clean_tiles","max_live_clean_region","wall_budget_12","wall_budget_8","matching_preparation_2","matching_preparation_0"]+
                    (["max_distinct_cuts_shared_pair","oracle_pair_best","oracle_reused_pair_best"] if a.guard_audit else []))
    for casefile in sorted(a.cases.glob("*.txt")):
        case=casefile.read_bytes()
        actions,err,rc,_=run([tester,solver],case,3.)
        match=re.fullmatch(rb"Score = (\d+)\s*",err)
        assert rc==0 and match
        with tempfile.TemporaryDirectory() as td:
            path=Path(td)/"recorded"
            trace=Path(td)/"trace"
            path.write_bytes(actions)
            out,error,ret,_=run([tester,sys.executable,harness,"--playback",path,trace],case,10.)
            assert ret==0 and out==actions and error==err
            lines=trace.read_text().splitlines()
        score,*_=replay(case.decode(),lines,actions.decode().splitlines(),detailed=True)
        assert score==int(match[1])
        _,hist,_=observed(case,lines)
        for x,y in (((4,4),) if a.guard_audit else LAYOUTS):
            writer.writerow([casefile.name,f"{x}x{y}",*analyze(layout(x,y),hist,a.guard_audit)])
        sys.stdout.flush()
if __name__=="__main__":main()
