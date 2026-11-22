"""Conditioned cow/pig exact short-lock probabilities on official traces."""
import argparse,csv,functools,itertools,re,sys,tempfile
from collections import Counter,defaultdict,deque
from pathlib import Path
from guarded_entry_trace import observed

N=30
V=N*N
ALL=(1<<V)-1
L=sum(1<<(i*N) for i in range(N))
R=L<<29
SPEED=(0,1,2,3,2,2)
B=[1<<i for i in range(V)]
def wave(x):return ((x&~R)<<1|(x&~L)>>1|x<<N|x>>N)&ALL
def idx(p):return 30*p[0]+p[1]
def cells(x):
    while x:
        y=x&-x;yield y.bit_length()-1;x-=y
ADJ=[tuple(cells(wave(b))) for b in B]
def flood(start,allowed):
    x=start
    while True:
        y=x|wave(x)&allowed
        if y==x:return x
        x=y
def geometry():
    at=[[] for _ in B]
    for x in range(N):
        for y in range(N):
            for h in range(1,min(24,N-x)+1):
                for w in range(1,min(24//h,N-y)+1):
                    if h*w<4:continue
                    g=sum((((1<<w)-1)<<(i*N+y)) for i in range(x,x+h))
                    z=(g,wave(g)&~g)
                    for p in cells(g):at[p].append(z)
    return at

def lock(inner,gates,open_grid,workers,others,speed,robust,record=False):
    """Exact probabilities for monotone shortest progress to any gate stance.

    Other humans are fixed and prevent building on occupied squares.
    Optimistic target-only lock ignores other animals after t0. A robust
    sufficient filter also bans gates whenever any other pet could reach
    gate adjacency by the turn, using distance before adding any new wall.
    """
    n=len(gates);full=(1<<n)-1
    poses=[]
    for p in gates:
        poses.append(set(u for u in ADJ[p] if open_grid&B[u] and not inner&B[u]))
    isgate={p:i for i,p in enumerate(gates)}
    banned=tuple(B[p]|wave(B[p]) for p in gates)
    stationary=tuple(p for i,p in enumerate(workers[0]) if i not in workers[1])
    initial=tuple(workers[0][i] for i in workers[1])
    distance=[]
    for stance in poses:
        d={}
        q=list(stance)
        for p in q:d[p]=0
        for u in q:
            if d[u]>=2:continue
            for v in ADJ[u]:
                if v not in d and v not in isgate and open_grid&B[v] and not B[v]&inner:
                    d[v]=d[u]+1;q.append(v)
        distance.append(d)
    near=[[False]*5 for _ in gates]
    for p,s in others:
        x=B[p]
        for t in range(5):
            if t:
                for _ in range(s):x|=wave(x)&open_grid
            for i,b in enumerate(banned):near[i][t]|=bool(x&b)
    cb=[sum(B[gates[i]] for i in range(n) if mask>>i&1) for mask in range(1<<n)]
    @functools.lru_cache(None)
    def outcome(p,closed):
        state={p:1.0}
        through=open_grid&~cb[closed]
        for _ in range(speed):
            nxt=defaultdict(float)
            for u,mass in state.items():
                v=[q for q in ADJ[u] if through&B[q]]
                for q in v:nxt[q]+=mass/len(v)
            state=nxt
        # End-of-pet-turn escape is absorbing. A pig can leave temporarily
        # and then re-enter within its two steps; use all intermediates.
        return tuple((u,mass) for u,mass in state.items() if inner&B[u])
    first={}
    @functools.lru_cache(None)
    def dp(h,start,p,closed,hs):
        if closed==full:return 1.0
        if h<=0:return 0.
        moves=[]
        ep=start-h
        occupied=set(hs)|set(stationary)
        for u in hs:
            choices=[(u,-1)]
            for i,g in enumerate(gates):
                if closed>>i&1:continue
                other_blocks=(robust or ep==0) and near[i][ep]
                if u in poses[i] and not B[p]&banned[i] and g not in occupied and not other_blocks:
                    choices.append((u,i))
            if h>1:
                for v in ADJ[u]:
                    if v in isgate or not open_grid&B[v] or inner&B[v]:continue
                    if any(not (closed>>i&1) and distance[i].get(v,99)<distance[i].get(u,99) for i in range(n)):
                        choices.append((v,-1))
            moves.append(choices)
        best=0.
        for x in itertools.product(*moves):
            mask=closed
            for _,i in x:
                if i>=0:mask|=1<<i
            if mask==full:
                if record and ep==0:first[(h,p)]=x
                return 1.
            if h<=1:continue
            dest=tuple(u for u,i in x)
            val=sum(pr*dp(h-1,start,u,mask,dest) for u,pr in outcome(p,mask))
            if val>best:
                best=val
                if record and ep==0:first[(h,p)]=x
        return best
    def compute(p):
        return [dp(h,h,p,0,initial) for h in (1,2,3,4)]
    compute.first=first
    return compute

def analyze(hist,kinds,at,times):
    raw=Counter(); rows=[]
    for t in times:
        pp,hh,walls=hist[t]
        hs=tuple(map(idx,hh)); ps=tuple(map(idx,pp))
        open_grid=ALL^sum(B[idx(w)] for w in walls)
        hbits=0
        for p in hs:hbits|=B[p]
        connected=flood(hbits,open_grid)
        for animal, (target,speed) in enumerate(zip(ps,kinds)):
            if speed not in (1,2) or not connected&B[target]:continue
            ts=Counter(); seen=set(); options=[]
            for g,bound in at[target]:
                req=bound&open_grid
                inner_open=g&open_grid
                if inner_open==g and req.bit_count()>3:continue
                real=g if inner_open==g else flood(B[target],inner_open)
                size=real.bit_count()
                if not 4<=size<=24:continue
                req=wave(real)&open_grid&~real
                n=req.bit_count()
                if not 1<=n<=3 or ((real|req)&hbits):continue
                integ=(real,req)
                if integ in seen:continue
                seen.add(integ)
                ts[f"geom{n}"]+=1
                if any(real&B[p] for i,p in enumerate(ps) if i!=animal):continue
                gs=tuple(cells(req))
                outside=open_grid&~(real|req)
                stance=tuple(wave(B[p])&outside for p in gs)
                if not all(stance):continue
                stancers=[[] for _ in gs]
                for i,h in enumerate(hs):
                    access=B[h]
                    for k in range(3):
                        for j,q in enumerate(stance):
                            if q&access and not any(x[1]==i for x in stancers[j]):stancers[j].append((k,i))
                        access|=wave(access)&outside
                if not all(stancers):continue
                ts[f"near{n}"]+=1
                eligible=sorted(set(i for a in stancers for k,i in a),key=lambda x:min(k for a in stancers for k,i in a if i==x))
                count= min(2,len(eligible))
                wsets=list(itertools.combinations(eligible,count))
                def key(w):
                    ds=[min((k for k,i in opts if i in w),default=99) for opts in stancers]
                    return max(ds),sum(ds)
                wsets.sort(key=key)
                if key(wsets[0])[0]>2:continue
                # Useful stats are on unique shape+target states, not count
                # of overlapping rectangle envelopes or every timestep.
                others=[(p,SPEED[k]) for i,(p,k) in enumerate(zip(ps,kinds)) if i!=animal]
                opt=[0.]*4; robust=[0.]*4
                others_adj=0
                for p,s in others:others_adj|=B[p]|wave(B[p])
                for w in wsets[:3]:
                    if key(w)[0]>2:continue
                    x=lock(real,gs,open_grid,(hs,w),others,speed,False)(target)
                    opt=list(map(max,opt,x))
                    if x[-1]>0:
                        y=lock(real,gs,open_grid,(hs,w),others,speed,True)(target)
                        robust=list(map(max,robust,y))
                options.append((size,gs,opt,robust,key(wsets[0]),target,req&others_adj,any(B[target]&wave(B[p]) for p in gs)))
            for k,v in ts.items():raw[(speed,k)]+=v
            for size,gs,opt,robust,dk,target,other,adj in options:
                rows.append((t,animal,speed,size,len(gs),'+'.join(map(str,gs)),target,dk[0],dk[1],bool(other),adj,*opt,*robust))
    return raw,rows

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','solver','cases','summary'):p.add_argument(n,type=Path)
    p.add_argument('--stride',type=int,default=1)
    p.add_argument('--max-cases',type=int)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester,solver=a.tester.resolve(),a.solver.resolve()
    script=a.harness.resolve()/'benchmark_cases.py'
    at=geometry()
    out=csv.writer(sys.stdout,lineterminator='\n')
    out.writerow(['case','t','animal','speed','area','missing','gates','position','worst_stance','sum_stance','other_blocks_now','animal_blocks_now',
                  'opt1','opt2','opt3','opt4','guard1','guard2','guard3','guard4'])
    with a.summary.open('w') as sf:
        summary=csv.writer(sf,lineterminator='\n')
        keys=list(itertools.product((1,2),('geom1','geom2','geom3','near1','near2','near3')))
        summary.writerow(['case']+[f'{s}_{k}' for s,k in keys])
        for f in sorted(a.cases.glob('*.txt'))[:a.max_cases]:
            case=f.read_bytes()
            cmd,err,rc,_=run([tester,solver],case,3.)
            match=re.fullmatch(rb'Score = (\d+)\s*',err)
            assert rc==0 and match
            with tempfile.TemporaryDirectory() as td:
                actions=Path(td)/'actions';actions.write_bytes(cmd)
                trace=Path(td)/'trace'
                actual,e,c,_=run([tester,sys.executable,script,'--playback',actions,trace],case,10.)
                assert c==0 and actual==cmd and e==err
                data=trace.read_text().splitlines()
            score,*_=replay(case.decode(),data,cmd.decode().splitlines(),detailed=True)
            assert score==int(match[1])
            kinds,h,_=observed(case,data)
            raw,rows=analyze(h,kinds,at,range(40,241,a.stride))
            summary.writerow([f.name,*[raw[k] for k in keys]])
            sf.flush()
            out.writerows((f.name,*r) for r in rows)
            sys.stdout.flush()
if __name__=='__main__':main()
