"""Read-only existing-rail transverse-pocket coverage on exact v24 tapes."""
from pathlib import Path
from collections import Counter
import csv,hashlib,json,sys,tempfile
W=Path(__file__).resolve().parent.parent
H=W/'ahc008_focus_solver/problems/ahc008'
sys.path[:0]=[str(H),str(H/'experiments/20261122'),str(W/'ahc008_bridge_audit')]
from bridge import parse_geometry
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,ADJ,idx,flood,wave
from parallel_banks_anchored_cut_audit import buf
from benchmark import replay
from benchmark_cases import run
T=W/'ahc008_focus_assets/tools/target/release/tester'
V=W/'ahc008_terry_same_geometry/hybrid/source-release/frozen'
F=W/'ahc008_v24_five_retention'
OWN={'U':-30,'D':30,'L':-1,'R':1,'u':-30,'d':30,'l':-1,'r':1}
def dest(p,c):return p+OWN[c]
def reach2(p,good):
    x=B[p];y=wave(x)&good
    return (x,y,wave(y)&good)
def options(w,ps,hs,gates,outside,act):
    legal=not buf(gates)&(buf(ps)|wave(buf(ps))|buf(hs))
    if not legal:return None
    dd=[]
    for gate in gates:
        stands=buf(q for q in ADJ[gate] if B[q]&outside)
        row=[]
        for i,p in enumerate(hs):
            # Existing unrelated walls and non-dot ordinary agency are not loaned.
            if act[i]!='.' or not B[p]&outside:row.append(99);continue
            ds=reach2(p,outside);row.append(next((r for r,x in enumerate(ds) if x&stands),99))
        dd.append(row)
    return maxetamin(dd)
def maxetamin(dd):
    return min(max(x,y) for i,x in enumerate(dd[0]) for j,y in enumerate(dd[1]) if i!=j)
def inspect(raw,moves,act,geom):
    kinds,hist,_=observed(raw,moves)
    st=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in hist]
    rot,_,tubes=parse_geometry(geom)
    by={(x.bank,x.row):x for x in tubes}
    axis=1 if rot else 30
    across=30 if rot else 1
    specs=[]
    for (bank,row),x in by.items():
        if not all(k in by for k in ((bank,row+2),(bank+1,row),(bank+1,row+2))):continue
        tr,bl,br=by[bank+1,row],by[bank,row+2],by[bank+1,row+2]
        assert x.r==tr.l and bl.r==br.l and bl.r==x.r+2*axis
        mid=x.r+axis
        specs.append((row,x.r,bl.r,(mid-across,mid,mid+across),(x,tr,bl,br)))
    last=st[-1]
    lastRoot=flood(buf(last[1]),ALL^last[2])
    missed={i for i,p in enumerate(last[0]) if B[p]&lastRoot}
    rows=[]; counts=Counter(); prepared={}
    for t in range(120,251):
        ps,hs,w=st[t];h=buf(hs);p=buf(ps); blocker=p|wave(p)|h
        root=flood(h,ALL^w)
        # Keep all previously alive rail stances, including unready rails.
        live=0
        for k in tubes:
            if not w&(k.gates|k.body):live|=B[k.l]|B[k.r]
        live &= root
        target={dest(x,q) for x,q in zip(hs,act[t]) if q.islower()}
        end=buf(dest(x,q) for x,q in zip(hs,act[t]) if q.isupper())
        heldW=buf(target)
        baseline_after_w=w|heldW
        for row,top,bottom,ch,four in specs:
            gates=top,bottom
            if w&buf(gates):continue
            locked=[bool(w&x.gates==x.gates) for x in four]
            # The two inner mouths are shared stances, so the remaining
            # original rail must be irrecoverably broken, not just unready.
            retire=[bool(w&(x.gates|x.body)) for x in four]
            already=None
            if sum(locked)==4:shape='natural';bounds=ch;plug=None
            elif sum(locked)==3:
                side=locked.index(False)%2
                plug=ch[0 if side==0 else 2];bounds=tuple(q for q in ch if q!=plug)
                already=bool(w&B[plug])
                shape=('old_plug' if already else 'new_plug') if all(retire) else 'destroys_live_rail'
            else:continue
            if buf(bounds)&w:continue
            planned=baseline_after_w|buf(gates)| (0 if plug is None else B[plug])
            outside=flood(B[hs[0]],ALL^planned)
            inside=flood(B[bounds[0]],ALL^planned)
            # Original moves remain on their actual exterior; protected live
            # rail stances and any existing legal wall command are respected.
            mustKeep=live&~buf(gates) if shape=='destroys_live_rail' else live
            if inside!=buf(bounds) or (h|end|mustKeep)&~outside or outside.bit_count()<300 or 5*outside.bit_count()<4*root.bit_count():continue
            gateMask=buf(gates)
            if (heldW|end)&gateMask:continue
            ids=[i for i,q in enumerate(ps) if B[q]&inside]
            # At live exact closure, pets legally away from both mouths.
            legalIds=ids if not blocker&gateMask else []
            useful=[i for i in legalIds if i in missed]
            time=options(w,ps,hs,gates,outside,act[t]) if legalIds else None
            pre=[]
            if shape=='new_plug' and t<=210 and not (blocker|heldW|end)&B[plug]:
                # Preparatory plug is not used in the unmodified tape.
                for i,x in enumerate(hs):
                    if act[t][i]=='.' and plug in ADJ[x] and B[x]&outside:
                        pre.append(i)
            counts[f'{shape}_geometry']+=1
            if not (ids or pre):continue
            former=prepared.get((row,plug))
            if pre:prepared.setdefault((row,plug),t)
            rows.append(dict(t=t,row=row,shape=shape,targets=ids,legal=legalIds,unresolved=useful,eta=time,
                             plug=plug,prebuild_now=pre,earliest_prior_prebuild=former,exterior=outside.bit_count(),mouths=list(gates)))
    return dict(missed=sorted(missed),counts=counts,opportunities=rows)
def main():
    out=Path(sys.argv[1]); cases=sorted((F/'m5-dev30/in').glob('*.txt'))
    ref={r['case']:r for r in csv.DictReader((F/'dev30.csv').open()) if r['model']=='baseline'}
    assert len(cases)==len(ref)==30
    rows=[]
    for f in cases:
        raw=f.read_bytes(); r=ref[f.name]
        assert r['input_sha256']==hashlib.sha256(raw).hexdigest()
        with tempfile.TemporaryDirectory() as directory:
            d=Path(directory); c=d/'cmd'; pets=d/'pets';geo=d/'geo'
            a,e,rc,_=run([str(T),'env','AHC008_BANK_LOG='+str(geo),str(V)],raw,4.5)
            assert rc==0 and hashlib.sha256(a).hexdigest()==r['actions_sha256'],f.name
            c.write_bytes(a)
            b,err,rc,_=run([str(T),sys.executable,H/'benchmark_cases.py','--playback',c,pets],raw,12)
            assert (b,err,rc)==(a,e,0)
            act=a.decode().splitlines(); moves=pets.read_text().splitlines()
            sf,*_=replay(raw.decode(),moves,act,detailed=True)
            assert sf==int(r['score']) and e.decode().strip()==f'Score = {sf}',(f.name,sf)
            rows.append({'case':f.name,**inspect(raw,moves,act,geo)})
        print(f.name,rows[-1]['counts'],flush=True)
    out.write_text(json.dumps(rows,indent=2)+'\n')
if __name__=='__main__':main()
