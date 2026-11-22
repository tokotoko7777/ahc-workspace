"""Only actual fence and guard readiness for diagonally shared exact-four pens."""
import argparse,csv,hashlib,json,sys,tempfile
from pathlib import Path
from guarded_entry_trace import observed
from quad_causal_trace import parse,xy
from stochastic_lock_audit import ALL,B,ADJ,idx,wave,cells,flood
from itertools import product
from collections import Counter
FIELDS=('geometry','geometry_inside','geometry_outside','reused_built_open_gate_turns','observed_body_pet_turns',
        'actual_body_clear_of_humans','original_gate_connected','actual_legal_two_or_fewer_walls','match_two_distinct_outside',
        'match_spare_two_distinct_outside','observed_distinct_pets','projected_large_fence_replacement',
        'ordinary_gate_eventually_walled')

def allshapes():
    shapes={((0,0),)}
    for _ in range(3):
        ss=set()
        for sh in shapes:
            for x,y in sh:
                for dx,dy in ((1,0),(-1,0),(0,1),(0,-1)):
                    z=(x+dx,y+dy)
                    if z in sh:continue
                    new=(*sh,z)
                    a=min(x for x,y in new);b=min(y for x,y in new)
                    ss.add(tuple(sorted((x-a,y-b) for x,y in new)))
        shapes=ss
    assert len(shapes)==19
    for sh in sorted(shapes):
        x=max(x for x,y in sh);y=max(y for x,y in sh)
        for r,c in product(range(30-x),range(30-y)):
            if r and c and r+x<29 and c+y<29:continue
            body=sum(B[(r+i)*30+c+j] for i,j in sh)
            yield body,wave(body)&~body

def match(pending,humans,inside,shell,spare=None):
    opt=[]
    for gate in cells(pending):
        cv=wave(B[gate])&~(inside|shell)
        opt.append([i for i,h in enumerate(humans) if cv&B[h] and (spare is None or spare[i])])
    def rec(k,used):
        if k==len(opt):return True
        return any(rec(k+1,used|{i}) for i in opt[k] if i not in used)
    return rec(0,set())

def examine(hist,log,kinds,cat):
    c=Counter()
    dog=log[0]
    orig=sum(B[idx(p)] for p in dog['dog_walls'])
    gate=dog['dog_gate']
    if not orig or gate<0:return c,[]
    gatebit=B[gate]
    walls=list(dog['dog_walls'])
    anchor=min(((0,0),(0,29),(29,0),(29,29)),key=lambda a:sum(abs(x-a[0])+abs(y-a[1]) for x,y in walls))
    an=B[idx(anchor)]
    far=B[idx((29-anchor[0],29-anchor[1]))]
    inside=flood(an,ALL&~(orig|gatebit))
    candidates=[]
    for body,shell in cat:
        reuse=shell&orig
        new=shell&~orig
        if not 1<=new.bit_count()<=2 or reuse.bit_count()<2 or body&orig or (body|shell)&(an|gatebit):continue
        # Require an exterior final door with both parity-safe positions.
        if not any(len({sum(xy(q))&1 for q in cells(body&~wave(B[g]))})==2 and
                   wave(B[g])&~(shell|body|orig) for g in cells(new)):continue
        outside=flood(an,ALL&~(orig|shell|body))
        if not outside&far:continue # dog gate remains usable in the physical ideal
        candidates.append((body,shell,reuse))
        c['geometry']+=1
        c['geometry_inside' if (body&inside)==body else 'geometry_outside']+=1
    distinct=set();witness=[]
    c['ordinary_gate_eventually_walled']=any(xy(gate) in w for _,_,w in hist)
    for t in range(70):
        pets,humans,walls=hist[t]
        ps=list(map(idx,pets));hs=list(map(idx,humans))
        b_p=sum({B[p] for p in ps});b_h=sum({B[h] for h in hs})
        blocked=b_p|wave(b_p)|b_h
        built=sum(B[idx(p)] for p in walls)
        if built&gatebit:continue
        for body,shell,reuse in candidates:
            if reuse&built!=reuse or body&built:continue
            c['reused_built_open_gate_turns']+=1
            inds=[i for i,p in enumerate(ps) if B[p]&body]
            if not inds:continue
            c['observed_body_pet_turns']+=1
            distinct.update(inds)
            if b_h&(body|shell):continue
            c['actual_body_clear_of_humans']+=1
            # Check the actual/finished fence geometry, not just ideal positions.
            reached=flood(an,ALL&~(built|orig|shell|body))
            orig_reach=flood(an,ALL&~(built|orig))
            if not reached&far or reached&b_h!=orig_reach&b_h:continue
            c['original_gate_connected']+=1
            need=shell&~built
            if not need or need&blocked:continue
            c['actual_legal_two_or_fewer_walls']+=1
            if not match(need,hs,body,shell):continue
            c['match_two_distinct_outside']+=1
            champs=list(log[t]['cmd'])
            spare=[c=='.' or (c.islower() and
                    hs[i]+dict(u=-30,d=30,l=-1,r=1)[c] in list(cells(need))) for i,c in enumerate(champs)]
            scheduled_humans=sum({B[idx(x)] for x in hist[t+1][1]})
            same=not scheduled_humans&(body|need) and match(need,hs,body,shell,spare)
            if same:c['match_spare_two_distinct_outside']+=1
            lack=sum(p not in inds and (B[x]&inside)!=0 for p,x in enumerate(ps))
            if same and not lack:c['projected_large_fence_replacement']+=1
            witness.append(dict(turn=t,pen=[xy(q) for q in cells(body)],added=[xy(q) for q in cells(need)],
                                pets=[dict(index=i,species=kinds[i]) for i in inds],spare=same,other_inside_large_fence=lack))
    c['observed_distinct_pets']=len(distinct)
    return c,witness

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','instrumented','cases','frozen','witness'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    key={r['case']:r for r in csv.DictReader(a.frozen.open()) if r['model']=='control'}
    geometry=list(allshapes())
    tester=a.tester.resolve();exe=a.instrumented.resolve()
    out=csv.writer(sys.stdout,lineterminator='\n');out.writerow(['case','dog_phase',*FIELDS])
    wl={}
    for f in sorted(a.cases.glob('*.txt')):
        case=f.read_bytes();row=key[f.name]
        with tempfile.TemporaryDirectory() as td:
            lg=Path(td)/'log'
            act,err,rc,_=run([tester,'env',f'AHC008_CAUSE_LOG={lg}',exe],case,3.)
            assert rc==0 and hashlib.sha256(act).hexdigest()==row['actions_sha256']
            assert int(err.split()[-1])==int(row['score'])
            fp=Path(td)/'a';fp.write_bytes(act);tr=Path(td)/'trace'
            a2,e2,r2,_=run([tester,sys.executable,a.harness.resolve()/'benchmark_cases.py','--playback',fp,tr],case,10.)
            assert (a2,e2,r2)==(act,err,0)
            txt=tr.read_text().splitlines()
            score,*_=replay(case.decode(),txt,act.decode().splitlines(),detailed=True)
            assert score==int(row['score'])
            kinds,hist,_=observed(case,txt);log=parse(lg.read_text())
        c,w=examine(hist,log,kinds,geometry)
        if w:wl[f.name]=w
        out.writerow([f.name,int(bool(log[0]['dog_walls'])),*[int(c[x]) for x in FIELDS]]);sys.stdout.flush()
    a.witness.write_text(json.dumps(wl,indent=2)+'\n')
if __name__=='__main__':main()
