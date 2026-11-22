"""Read-only existing service road hard-anchored compact cut feasibility."""
import argparse,csv,hashlib,json,sys,tempfile
from collections import Counter
from pathlib import Path
import numpy as np
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import maximum_flow
from guarded_entry_trace import observed
from stochastic_lock_audit import ADJ,B,ALL,idx,flood,wave
from parallel_banks_step_separator_audit import bits,distances,parts,buf,rect

END=1801; START=1800; INF=100_000_000
OFFSETS=(0,8,16)


def add_route(vertices, p, can):
    d=distances(vertices,can)
    assert d[p]>=0
    connect=B[p]
    while not vertices&B[p]:
        p=min(x for x in ADJ[p] if d[x]>=0 and d[x]==d[p]-1)
        connect|=B[p]
    return vertices|connect


def protect(lanes, rot, root, humans):
    board=buf((c*30+r if rot else r*30+c) for r in range(30) for c in range(30)
              if r in (0,29) or c in lanes)&root
    if not board:return 0,0
    chunks=sorted(parts(board),key=lambda b:(-b.bit_count(),b))
    home=chunks[0]
    for side in chunks[1:]:
        d=distances(home,root)
        p=min(bits(side),key=lambda q:(d[q],q))
        home=add_route(home,p,root)|side
    for h in humans:
        if not home&B[h]:home=add_route(home,h,root)
    assert home&board==board and flood(home&-home,home)==home
    return home,board


def prep_cut(root,anchor,legal,near):
    xx=[];yy=[];ww=[]
    def arc(a,c,w):xx.append(a);yy.append(c);ww.append(w)
    for p in bits(root):
        u,v=2*p,2*p+1
        d=min((near[q] for q in ADJ[p] if near[q]>=0),default=900)
        arc(u,v,1_000_000+16*d if legal&B[p] and not anchor&B[p] else INF)
        if anchor&B[p]:arc(START,u,INF)
        for q in ADJ[p]:
            if root&B[q]:arc(v,2*q,INF)
    return csr_matrix((np.array(ww,dtype=np.int64),(xx,yy)),shape=(END+1,END+1))


def vertex_cut(net,root,target):
    supply=csr_matrix((np.array([INF],dtype=np.int64),([2*target+1],[END])),shape=net.shape)
    conn=net+supply
    f=maximum_flow(conn,START,END)
    if f.flow_value>=INF:return 0
    r=(conn-f.flow).tocsr()
    reached={START};stack=[START]
    for u in stack:
        i,j=r.indptr[u:u+2]
        for q,c in zip(r.indices[i:j],r.data[i:j]):
            if c>0 and q not in reached:reached.add(q);stack.append(q)
    return buf(p for p in bits(root) if 2*p in reached and 2*p+1 not in reached)


def staffing(kept,cut,humans,cap=6):
    crew=[distances(B[h],kept) for h in humans]
    choices=[]
    for p in bits(cut):
        opts=[]
        for w,d in enumerate(crew):
            candidate=min(((d[q],q) for q in ADJ[p] if d[q]>=0),default=None)
            if candidate is not None and candidate[0]<=cap:opts.append((candidate[0],w,candidate[1]))
        choices.append((p,opts))
    choices.sort(key=lambda x:len(x[1]))
    def at(i,used,lim):
        if i==len(choices):return []
        p,opts=choices[i]
        for d,w,q in opts:
            if d>lim or used>>w&1:continue
            z=at(i+1,used|1<<w,lim)
            if z is not None:return [(p,w,q,d)]+z
        return None
    for eta in range(cap+1):
        z=at(0,0,eta)
        if z is not None:return eta,z
    return None,None


def event(hist,types,case,t,rot,lanes,misses):
    p0,h0,w=hist[t]
    pp=list(map(idx,p0));hh=list(map(idx,h0))
    p=buf(pp);h=buf(hh)
    opened=ALL^buf(map(idx,w));root=flood(B[hh[0]],opened)
    main=dict(kind='snapshot',case=case,turn=t,area=root.bit_count())
    if h&root!=h:return [{**main,'status':'already_split'}]
    a,orig=protect(lanes,rot,root,hh)
    if not a:return [{**main,'status':'no_open_roads'}]
    visible=[i for i,q in enumerate(pp) if root&B[q]]
    movable=[i for i in visible if not a&B[pp[i]]]
    out=[{**main,'status':'anchored','roads':orig.bit_count(),'connected_anchor':a.bit_count(),
          'total_reachable':len(visible),'off_anchor':len(movable)}]
    if not movable:return out
    now=root&~(h|p|wave(p)); potential=root&~(h|p)
    near=distances(h,root)
    road_dist=distances(a,root)
    strict=prep_cut(root,a,now,near)
    geom=prep_cut(root,a,potential,near)
    for pet in movable:
        for tag,net in [('strict',strict),('geometric',geom)]:
            target=pp[pet]
            ans=vertex_cut(net,root,target)
            row=dict(kind='pet',case=case,turn=t,pet=pet,species=types[pet],mode=tag,
                     normally_unresolved=int(pet in misses),anchor_distance=road_dist[target],human_distance=near[target])
            if not ans or ans.bit_count()>7:
                out.append({**row,'status':'no_cut_at_most_7'});continue
            after=root&~ans
            pen=flood(B[target],after);kept=flood(a&-a,after)
            assert a&kept==a and h&kept==h and not pen&h
            if pen.bit_count()>48:
                out.append({**row,'status':'pen_over_48','pen_area':pen.bit_count()});continue
            # Reopen far-side cut cells that do not touch the kept side.
            needed=ans&wave(kept)
            if needed!=ans:
                after=root&~needed
                assert flood(a&-a,after)==kept
                pen=flood(B[target],after)
                if pen.bit_count()>48:
                    out.append({**row,'status':'pen_over_48','pen_area':pen.bit_count()});continue
            row.update(status='compact',pen_area=pen.bit_count(),nonrect=int(not rect(pen)),
                pen=list(bits(pen)),walls=list(bits(needed)),jobs=needed.bit_count(),kept=kept.bit_count(),
                still_blocked=(needed&~now).bit_count(),
                enclosed_pets=[i for i,q in enumerate(pp) if pen&B[q]])
            eta,crew=staffing(kept,needed,hh)
            row.update(crew_eta=eta,builder_route=crew)
            out.append(row)
    return out


def summarize(out,verified):
    snaps=[r for r in out if r['kind']=='snapshot']
    ans=dict(verified=verified,snapshots=len(snaps),
        snapshot_status=dict(Counter(r['status'] for r in snaps)),
        off_anchor=sum(r.get('off_anchor',0) for r in snaps),result={})
    for mode in ('strict','geometric'):
        q=[r for r in out if r['kind']=='pet' and r['mode']==mode]
        for name,p in [('all',lambda r:True),('compact',lambda r:r['status']=='compact'),
          ('nonrect',lambda r:r.get('nonrect',0)),
          ('all_now_legal_nonrect',lambda r:r.get('nonrect',0) and not r['still_blocked']),
          ('routed',lambda r:r.get('nonrect',0) and not r['still_blocked'] and r['crew_eta'] is not None),
          ('routed_immediate',lambda r:r.get('nonrect',0) and not r['still_blocked'] and r['crew_eta']==0)]:
            sub=[r for r in q if p(r)]
            ans['result'][mode+'_'+name]=dict(events=len(sub),cases=len({r['case'] for r in sub}),
                pets=len({(r['case'],r['pet']) for r in sub}),
                ordinarily_missed=len({(r['case'],r['pet']) for r in sub if r['normally_unresolved']}),
                missed_cases=len({r['case'] for r in sub if r['normally_unresolved']}))
    return ans


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','solver','inputs','control','out','summary'):p.add_argument(n,type=Path)
    a=p.parse_args();sys.path.insert(0,str(a.harness.resolve()))
    from benchmark import replay
    from benchmark_cases import run
    with a.control.open() as f:ref=[r for r in csv.DictReader(f) if r['model']=='narrow']
    out=[]
    for r in ref:
        name=r['case'];case=(a.inputs/name).read_bytes();release=int(r['release'])
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);meta=d/'geo';act=d/'act';trace=d/'trace'
            test=a.tester.resolve()
            stdout,e,rc,_=run([test,'env',f'AHC008_BANK_LOG={meta}',a.solver.resolve()],case,3.)
            assert rc==0 and e.strip()==('Score = '+r['score']).encode()
            assert r['actions_sha256']==hashlib.sha256(stdout).hexdigest()
            act.write_bytes(stdout)
            pb,e2,rc2,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                             '--playback',act,trace],case,10.)
            assert (pb,e2,rc2)==(stdout,e,0)
            lines=trace.read_text().splitlines()
            assert replay(case.decode(),lines,stdout.decode().splitlines(),detailed=True)[0]==int(r['score'])
            kinds,hist,_=observed(case,lines)
            geom=meta.read_text().splitlines();rot,shift,_=map(int,geom[0].split())
            n=max(int(x.split()[0]) for x in geom[1:])+1
            lanes=[shift+7*k for k in range(n+1)]
            ps,hs,w=hist[-1]
            final=flood(buf(map(idx,hs)),ALL^buf(map(idx,w)))
            missed={i for i,x in enumerate(ps) if final&B[idx(x)]}
            for d in OFFSETS:out.extend(event(hist,kinds,name,release+d,rot,lanes,missed))
        print(name,flush=True)
    a.out.write_text(''.join(json.dumps(r,separators=(',',':'))+'\n' for r in out))
    s=summarize(out,len(ref));a.summary.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))
def fixture():
    pts=lambda pairs:buf(r*30+c for r,c in pairs)
    anchor=pts([(10,c) for c in range(5,9)])
    root=anchor|pts([(11,8),(12,8),(12,9),(12,10),(13,10),(13,11),(14,11)])
    humans=[308];target=431
    now=root&~(B[humans[0]]|B[target]|wave(B[target]))
    net=prep_cut(root,anchor,now,distances(B[humans[0]],root))
    cut=vertex_cut(net,root,target)
    assert cut==B[338]
    assert flood(B[target],root&~cut).bit_count()==6
    assert not rect(flood(B[target],root&~cut))
    assert staffing(anchor,cut,humans)[0]==0

if __name__=='__main__':
    if sys.argv[1:]==['--fixture']:fixture()
    else:main()
