"""Exact end-state + static legal route audit of stepped band separators, no rollout."""
import argparse,csv,hashlib,itertools,json,sys,tempfile
from pathlib import Path
from collections import defaultdict
import numpy as np
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import maximum_flow
from guarded_entry_trace import observed
from stochastic_lock_audit import ADJ,ALL,B,idx,wave,flood

W=30;V=900;INF=100_000_000
MAX_JOBS=18;MAX_ROUTE=12;MIN_KEEP=300;MAX_EVAC=6
def bits(mask):
    while mask:
        k=mask&-mask;yield k.bit_length()-1;mask^=k
def buf(points):
    return sum({B[p] for p in points})
def distances(start,open_):
    result=[-1]*V
    front=seen=start&open_;d=0
    while front:
        for p in bits(front):result[p]=d
        front=wave(front)&open_&~seen;seen|=front;d+=1
    return result
def parts(open_):
    while open_:
        k=flood(open_&-open_,open_)
        yield k;open_&=~k
def rect(comp):
    xy=list(map(lambda p:divmod(p,30),bits(comp)))
    if not xy:return False
    x,y=zip(*xy)
    return (max(x)-min(x)+1)*(max(y)-min(y)+1)==comp.bit_count()

def legal_graph(history,geometry,t):
    pets,humans,walls=history[t]
    rot,shift,_=map(int,geometry[0].split())
    geom=[[int(p) for p in line.split()] for line in geometry[1:]]
    braces=buf(p for r in geom for p in r[10:])
    starting=buf(map(idx,humans));opened=ALL^buf(map(idx,walls))
    root=flood(B[idx(humans[0])],opened)
    assert starting&root==starting
    assert not braces & (starting&~opened)
    pet=[idx(p) for p in pets]
    pet_bit=buf(pet)
    unavailable=wave(pet_bit)|pet_bit|starting
    crew=[idx(p) for p in humans]
    return dict(root=root,legal=root&~unavailable,near=distances(starting,root),
                crew=crew,pets=pet,braces=braces&~opened,closed=ALL^opened,rot=rot,shift=shift)

def structural_cut(ctx,center,keep_top,force):
    """A vertex separator of two exterior strips; any missing walls in band only."""
    root,near=ctx['root'],ctx['near']
    row=lambda p: p%30 if ctx['rot'] else p//30
    lo,hi=center-4,center+4
    s,t=2*V,2*V+1
    from_,to,cap=[],[],[]
    def edge(a,b,c):from_.append(a);to.append(b);cap.append(c)
    pet=ctx['pets']
    force_pets={p for p in pet if root&B[p] and (row(p)>center if keep_top else row(p)<center)} if force else set()
    for p in bits(root):
        r=row(p); u,v=2*p,2*p+1
        can=lo<r<hi and ctx['legal']&B[p]
        d=min((near[q] for q in ADJ[p] if near[q]>=0),default=900)
        edge(u,v,4096+8*d if can else INF)
        if (r<=lo if keep_top else r>=hi):edge(s,u,INF)
        if (r>=hi if keep_top else r<=lo) or p in force_pets:edge(v,t,INF)
        for q in ADJ[p]:
            if root&B[q]:edge(v,2*q,INF)
    A=csr_matrix((np.array(cap,dtype=np.int64),(from_,to)),shape=(t+1,t+1))
    fl=maximum_flow(A,s,t)
    if fl.flow_value>=INF:return None
    graph=(A-fl.flow).tocsr()
    seen={s};todo=[s]
    for u in todo:
        a,b=graph.indptr[u:u+2]
        for q,c in zip(graph.indices[a:b],graph.data[a:b]):
            if c>0 and q not in seen:seen.add(q);todo.append(q)
    cut=buf(p for p in bits(root) if 2*p in seen and 2*p+1 not in seen)
    if not cut:return None
    return cut,{'center':center,'keep_top':keep_top,'force_visible':force}

def line_sequences():
    for dx,dy in [(0,1),(1,-1),(1,0),(1,1)]:
        for x in range(30):
            for y in range(30):
                if 0<=x-dx<30 and 0<=y-dy<30:continue
                row=[];r,c=x,y
                while 0<=r<30 and 0<=c<30:
                    row.append(r*30+c);r+=dx;c+=dy
                if len(row)>=3:yield row
LINES=list(line_sequences())

def near_boardwall(ctx):
    man=ctx['closed']
    # Exact grid 8-neighbors (no axial distance-two additions).
    h=((man&~sum(1<<(30*i+29) for i in range(30)))<<1) | ((man&~sum(1<<(30*i) for i in range(30)))>>1)
    neighbor=(h|(man|h)<<30|(man|h)>>30)&ALL
    border=buf(p for p in range(V) if p//30 in (0,29) or p%30 in (0,29))
    return neighbor|border

def straight(ctx):
    n=near_boardwall(ctx)
    root,legal=ctx['root'],ctx['legal']
    keep={}
    for line in LINES:
        for i,p in enumerate(line):
            if not legal&B[p] or not n&B[p]:continue
            w=0;shape=[]
            for q in line[i:i+MAX_JOBS]:
                if not (root&B[q] and legal&B[q]):break
                w|=B[q];shape.append(q)
                if n&B[q]:keep.setdefault(w,tuple(shape))
    return keep

def wiggles(shape,ctx):
    n=near_boardwall(ctx)
    legal=ctx['legal']
    is8=lambda a,b: max(abs(a//30-b//30),abs(a%30-b%30))<=1 and a!=b
    for i in range(len(shape)):
        for ln in range(1,min(3,len(shape)-i)+1):
            for dx,dy in ((0,1),(0,-1),(1,0),(-1,0)):
                x=list(shape)
                for j in range(i,i+ln):
                    r,c=divmod(shape[j],30);r+=dx;c+=dy
                    if not (0<=r<30 and 0<=c<30):break
                    q=r*30+c
                    if not legal&B[q]:break
                    x[j]=q
                else:
                    if len(set(x))!=len(x) or not (n&B[x[0]] and n&B[x[-1]]):continue
                    if i and not is8(x[i-1],x[i]):continue
                    if i+ln<len(x) and not is8(x[i+ln-1],x[i+ln]):continue
                    yield buf(x),tuple(x)

def assess(cut,ctx):
    if not cut or cut.bit_count()>MAX_JOBS:return []
    fp=[(i,p) for i,p in enumerate(ctx['pets']) if ctx['root']&B[p]]
    open_=ctx['root']&~cut
    groups=list(parts(open_))
    if len(groups)<2:return []
    good=[]
    for keep in groups:
        a=keep.bit_count()
        if a<MIN_KEEP:continue
        outside=[i for i,p in fp if not keep&B[p]]
        inside=[i for i,p in fp if keep&B[p]]
        if not outside:continue
        value=a*(.5**len(inside))
        # Opening proposed vertices not adjacent to the actual retained
        # component cannot restore any path out of it. Liberate crews from
        # redundant far-side jobs and rematerialize incident pet components.
        required=cut&wave(keep)
        if not required:continue
        current=ctx['root']&~required
        assert flood(keep&-keep,current)==keep
        retained_partition=groups if required==cut else list(parts(current))
        groups_out=[]
        for comp in retained_partition:
            if comp==keep:continue
            ids=[i for i,p in fp if comp&B[p]]
            if ids:groups_out.append(dict(area=comp.bit_count(),nonrect=int(not rect(comp)),pets=ids))
        all_stances=all(any(keep&B[q] for q in ADJ[p]) for p in bits(required))
        # Geometric score by area per person on one retained side.
        good.append(dict(area=a,pets_kept=inside,detached=outside,components=groups_out,
                  geometric_score=value,stance_support=int(all_stances),cut=required,keep=keep,origin_cut=cut))
    return good

def route(opts,ctx):
    keep=opts['keep'];cut=opts['cut'];crew=ctx['crew'];m=len(crew)
    if not opts['stance_support']:return {},'no_stance'
    d=distances(keep,ctx['root'])
    start=[]
    for p in crew:
        if d[p]<0 or d[p]>MAX_EVAC:return {},'evac'
        while not keep&B[p]:p=min(q for q in ADJ[p] if d[q]==d[p]-1 and d[q]>=0)
        start.append(p)
    elapsed=[max(d[p] for p in crew)]*m
    memo={}; pos=list(start);todo=set(bits(cut));sched=[]
    while todo:
        pair=[]
        for wall in todo:
            options=[]
            for w,p in enumerate(pos):
                if p not in memo:memo[p]=distances(B[p],keep)
                dd=memo[p]
                dt,st=min((dd[q],q) for q in ADJ[wall] if keep&B[q])
                options.append((elapsed[w]+dt+1,wall,w,st))
            pair.append(min(options))
        # First schedule the hard jobs (best possible current finish is latest).
        # This is a constructive bound, not the optimal feasible route.
        t,p,w,q=max(pair,key=lambda a:(a[0],-a[1]))
        if t>MAX_ROUTE:return {},'route'
        sched.append((t,w,p,q))
        elapsed[w]=t;pos[w]=q;todo.remove(p)
    return dict(max_evac=max(d[p] for p in crew),finish=max(elapsed),
                evacuation_entries=start,build_schedule=sorted(sched)),'pass'

def run_snapshot(ctx):
    initial=ctx['root'].bit_count()*(.5**sum(bool(ctx['root']&B[p]) for p in ctx['pets']))
    old=straight(ctx); all_eval={}; shapes=defaultdict(list)
    def calc(cut):
        if cut not in all_eval:all_eval[cut]=[q for q in assess(cut,ctx) if q['geometric_score']>initial]
        return all_eval[cut]
    def best_geom(masks):
        unique={}
        for c in masks:
            for q in calc(c):
                unique.setdefault((q['cut'],q['keep']),q)
        return sorted(unique.values(),key=lambda q:(-q['geometric_score'],q['cut'].bit_count(),q['cut']))
    old_vals=best_geom(old)
    tested=set(old);wing={}
    # Match a conservative legacy-style top-4, two 1–3-cell-shift rounds.
    initial_heads=[];seen=set()
    for v in old_vals:
        c=v['origin_cut']
        if c not in seen:initial_heads.append(c);seen.add(c)
        if len(initial_heads)>=4:break
    for c in initial_heads:
        head=old[c]
        for _ in range(2):
            best=0;updated=None
            for w,x in wiggles(head,ctx):
                if w in tested:continue
                tested.add(w)
                for g in calc(w):
                    wing[w]=x
                    if g['geometric_score']>best:best=g['geometric_score'];updated=x
            if updated is None:break
            head=updated
    new={}
    for center,top,force in itertools.product((8,12,16,20,24),(True,False),(False,True)):
        res=structural_cut(ctx,center,top,force)
        if res:
            w,a=res
            if w.bit_count()<=MAX_JOBS:new.setdefault(w,a)
    results={'candidate_masks':{'straight':len(old),'wiggle':len(wing),'stepped':len(new)}}
    results['initial_area']=ctx['root'].bit_count()
    results['initial_value']=initial
    results['old_score_geometric']=[v['geometric_score'] for v in old_vals[:1]]
    legacy=set(old)|set(wing)
    legacy_masks={v['cut'] for v in best_geom(legacy)}
    for name,pool in [('legacy',legacy),('stepped',new)]:
        evals=best_geom(pool)
        val=[];fail=defaultdict(int)
        geom_clean=[x for x in evals if not x['pets_kept']]
        for v in evals:
            path,reason=route(v,ctx)
            fail[reason]+=1
            if path:
                cut=v['cut'];bb=v['keep']
                meta=dict(v)
                del meta['keep'];del meta['cut'];del meta['origin_cut']
                meta.update(path)
                meta.update(jobs=cut.bit_count(),walls=list(bits(cut)),reachable_area=list(bits(bb)),
                            brace_frontier=(wave(bb)&wave(ctx['root']&~(bb|cut))&ctx['braces']).bit_count(),
                            new_missing_wall_shape=int(cut not in legacy_masks))
                if name=='stepped':meta.update(new[v['origin_cut']])
                val.append(meta)
                break
        geometric_witness=None
        if geom_clean:
            q=geom_clean[0]
            geometric_witness=dict(keeper_area=q['area'],pet_components=q['components'],pet_ids=q['detached'],
                                   walls=list(bits(q['cut'])),keepers=list(bits(q['keep'])))
            geometric_witness['route_reject']=route(q,ctx)[1]
        results[name]=dict(geometric=len(evals),best_geom=evals[0]['geometric_score'] if evals else 0,
                           clean_geometry_witness=geometric_witness,
                           understaffed_sizes=[v['cut'].bit_count() for v in evals[:3]],
                           precise_geometric_clean=[(v['area'],v['cut'].bit_count(),len(v['detached'])) for v in geom_clean[:3]],
                           failure_before_best=dict(fail),
                           routed=val[0] if val else None)
    return results

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','solver','cases','control','output'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with a.control.open() as f:ref=[r for r in csv.DictReader(f) if r['model']=='narrow']
    out={}
    for r in ref:
        name=r['case'];case=(a.cases/name).read_bytes()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);met=d/'meta'
            cmd=[a.tester.resolve(),'env',f'AHC008_BANK_LOG={met}',a.solver.resolve()]
            acts,e,rc,_=run(cmd,case,3.)
            assert rc==0 and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(acts).hexdigest()==r['actions_sha256']
            af=d/'actions';tr=d/'trace';af.write_bytes(acts)
            play,e2,rc2,_=run([a.tester.resolve(),sys.executable,a.harness.resolve()/'benchmark_cases.py',
                             '--playback',af,tr],case,10.)
            assert (play,e2,rc2)==(acts,e,0)
            text=tr.read_text().splitlines()
            assert int(r['score'])==replay(case.decode(),text,acts.decode().splitlines(),detailed=True)[0]
            species,history,_=observed(case,text)
            t=int(r['release'])
            ctx=legal_graph(history,met.read_text().splitlines(),t)
            out[name]=run_snapshot(ctx)
            out[name].update(turn=t,species=species)
            print(name,*(bool(out[name][k]['routed']) for k in ('legacy','stepped')),flush=True)
        a.output.write_text(json.dumps(out,indent=2)+'\n')
if __name__=='__main__':main()
