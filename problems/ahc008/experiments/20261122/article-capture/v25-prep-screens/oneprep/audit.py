"""One off-rail preparatory wall, one later mouth; factual coverage only."""
from pathlib import Path
import collections,csv,hashlib,json,sys,tempfile
W=Path(__file__).resolve().parent.parent
H=W/'ahc008_focus_solver/problems/ahc008'
sys.path[:0]=[str(H),str(H/'experiments/20261122'),str(W/'ahc008_bridge_audit')]
from bridge import parse_geometry
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,ADJ,idx,flood,wave
from parallel_banks_anchored_cut_audit import buf,bits
from benchmark import replay
from benchmark_cases import run
T=W/'ahc008_focus_assets/tools/target/release/tester'
R=W/'ahc008_v24_standby_early_seam'
D={'U':-30,'D':30,'L':-1,'R':1,'u':-30,'d':30,'l':-1,'r':1}
sys.setrecursionlimit(3000)
def points(h,act):
    builds=buf(x+D[c] for x,c in zip(h,act) if c.islower())
    dest=buf(x+D[c] if c.isupper() else x for x,c in zip(h,act))
    return builds,dest
def idle_routes(h,act,allowed,d=2):
    for i,p in enumerate(h):
        if act[i]!='.' or not B[p]&allowed:continue
        q=B[p]; yield i,[q]+[(q:=wave(q)&allowed) for _ in range(d)]
def articulation(h,op):
    """Candidate (second vertex, potential chamber size) after prep."""
    tick=0; seen=[-1]*900; low=[0]*900; found=set()
    def dfs(p,prev):
        nonlocal tick
        seen[p]=low[p]=tick;tick+=1
        count=1
        for q in ADJ[p]:
            if q==prev or not op&B[q]:continue
            if seen[q]>=0:low[p]=min(low[p],seen[q]);continue
            s=dfs(q,p);count+=s;low[p]=min(low[p],low[q])
            if low[q]>=seen[p] and 4<=s<=64:found.add(p)
        return count
    dfs(h,-1)
    return found
def audit(raw,replies,actions,geo):
    kinds,hist,_=observed(raw,replies)
    states=[(tuple(map(idx,p)),tuple(map(idx,h)),buf(map(idx,w))) for p,h,w in hist]
    rot,shift,tubes=parse_geometry(geo)
    pf=lambda r,c:c*30+r if rot else r*30+c
    roads=buf(pf(r,c) for r in range(30) for c in (shift,shift+7,shift+14))
    footprint=roads
    for x in tubes:footprint|=x.body|x.gates|x.wall|B[x.l]|B[x.r]
    lastp,lasth,lastw=states[-1]
    final=flood(buf(lasth),ALL^lastw)
    missed={i for i,p in enumerate(lastp) if final&B[p]}
    cuts=[]; counts=collections.Counter()
    for t in (160,180,200):
        ps,hs,w=states[t]; origActs=actions[t]; hb,pb=buf(hs),buf(ps)
        old=flood(B[hs[0]],ALL^w)
        builds,dest=points(hs,origActs)
        post=old&~builds
        forbidden=footprint|w|builds|dest|hb|pb|wave(pb)
        if (hb|dest)&~old:continue
        prep={}
        for i,ds in idle_routes(hs,origActs,post,d=1):
            for eta,poses in enumerate(ds):
                for p in bits((wave(poses)&old)&~forbidden):
                    if eta==1 and p==hs[i]:continue
                    pose=min(q for q in ADJ[p] if poses&B[q])
                    if p not in prep or (eta,i,pose)<prep[p]:prep[p]=(eta,i,pose)
        counts['prepared_cells']+=len(prep)
        live=buf(q for x in tubes for q in (x.l,x.r) if not w&(x.body|x.gates))
        must=(roads|live)&old
        for p,(eta,actor,pose) in prep.items():
            afterprep=post&~B[p]
            exterior=flood(B[hs[0]],afterprep)
            # No prep-only split of any old actual post-command cells.
            if (must|hb|dest|B[pose])&~exterior or exterior!=afterprep:continue
            counts['safe_prep_cells']+=1
            for g in sorted(articulation(hs[0],exterior)):
                if not (exterior&B[g]) or footprint&B[g] or hb&B[g] or dest&B[g]:continue
                mask=exterior&~B[g]; safe=flood(B[hs[0]],mask)
                chamber=mask&~safe
                if safe.bit_count()<300 or 4*safe.bit_count()<3*old.bit_count():continue
                if (hb|dest|must)&~safe:continue
                if not 4<=chamber.bit_count()<=64:continue
                if flood(chamber&-chamber,chamber)!=chamber:continue
                if not any(B[q]&safe for q in ADJ[g]):continue
                # Check that closing just the mouth had not already detached
                # this chamber. Both walls must genuinely be needed.
                alone=flood(B[hs[0]],post&~B[g])
                if chamber&~alone:continue
                histwin=[];visitors=set()
                for u in range(t+eta+1,min(260,t+45)+1):
                    qs,j,ww=states[u]; jp=buf(qs); jh=buf(j)
                    if ww&B[g]:break
                    if not (jp|jh)&B[p]:
                        bb,ee=points(j,actions[u])
                        pp=ww|bb|B[p]
                        void=buf([p,g])
                        if (bb|ee|jh)&void:continue
                        lab=flood(B[j[0]],ALL^(pp|B[g]))
                        pocket=chamber&~pp
                        if not pocket or (jh|ee)&~lab:continue
                        real=flood(pocket&-pocket,ALL^(pp|B[g]))
                        if real!=pocket or not 4<=real.bit_count()<=64:continue
                        here=flood(B[j[0]],ALL^ww)
                        current_stances=buf(q for x in tubes for q in (x.l,x.r) if not ww&(x.body|x.gates))
                        if ((roads|current_stances)&here)&~lab:continue
                        if lab.bit_count()<300 or 4*lab.bit_count()<3*here.bit_count():continue
                        ids=[i for i,q in enumerate(qs) if i in missed and real&B[q]]
                        visitors.update(ids)
                        if not ids or (jp|wave(jp))&B[g]:continue
                        stands=buf(q for q in ADJ[g] if lab&B[q])
                        options=[(d,i) for i,r in idle_routes(j,actions[u],lab) for d,b in enumerate(r) if b&stands]
                        histwin.append((u,ids,min(options,default=(99,-1))))
                cuts.append(dict(turn=t,prep=p,prepare_eta=eta,actor=actor,prep_stance=pose,gate=g,
                              room=chamber.bit_count(),floor=safe.bit_count(),visitors=sorted(visitors),windows=histwin))
    return dict(counts=counts,cuts=cuts)
def main():
    out=Path(sys.argv[1]);cases=sorted((W/'ahc008_v24_five_retention/m5-dev30/in').glob('*.txt'))
    ref={r['case']:r for r in csv.DictReader((R/'dev30.csv').open()) if r['model']=='article'}
    assert len(cases)==len(ref)==30
    rows=[]
    for f in cases:
        raw=f.read_bytes(); r=ref[f.name]
        assert hashlib.sha256(raw).hexdigest()==r['input_sha256']
        with tempfile.TemporaryDirectory() as q:
            d=Path(q); a=d/'cmd'; z=d/'reply'; geo=d/'geo'
            act,e,rc,_=run([str(T),'env','AHC008_BANK_LOG='+str(geo),str(R/'solve')],raw,4.5)
            assert rc==0 and hashlib.sha256(act).hexdigest()==r['actions_sha256'],f.name
            a.write_bytes(act)
            b,err,rc,_=run([str(T),sys.executable,H/'benchmark_cases.py','--playback',a,z],raw,12.)
            assert (b,err,rc)==(act,e,0)
            actions=act.decode().splitlines(); replies=z.read_text().splitlines()
            score,*_=replay(raw.decode(),replies,actions,detailed=True)
            assert score==int(r['score']) and e.decode().strip()==f'Score = {score}'
            rows.append(dict(case=f.name,exact=True,score=score,**audit(raw,replies,actions,geo)))
        print(f.name,rows[-1]['counts'],len(rows[-1]['cuts']),flush=True)
    out.write_text(json.dumps(rows,indent=2)+'\n')
if __name__=='__main__':main()
