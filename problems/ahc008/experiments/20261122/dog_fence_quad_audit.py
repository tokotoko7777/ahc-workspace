"""Only actual fence and guard readiness for diagonally shared exact-four pens."""
import argparse,csv,hashlib,json,sys,tempfile
from pathlib import Path
from guarded_entry_trace import observed,STEPS,step
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

def examine(hist,log,kinds,cat,trace=None,details=False):
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
    if details:return route_windows(hist,log,kinds,candidates,trace,inside,orig,gate)
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

ROUTES=('route_phase_turns','geometry_stance_visits','ready_legal_single_jobs','legal_single_idle',
        'finish_gate_pet_windows','paired_by8','paired_with_no_traced_crossing',
        'paired_idle_both','paired_no_other_pet_in_large_corner','paired_never_ordinary_gate')
def route_windows(hist,log,kinds,candidates,trace,parent,fence,gate):
    out=Counter();witness=[]
    steps=[]
    for t in range(300):
        pbits=0
        for p,moves in zip(hist[t][0],trace[2*t+1].split()):
            for ch in moves:
                if ch=='.':continue
                p=step(p,STEPS[ch])
                pbits|=B[idx(p)]
        steps.append(pbits)
    qualified=[]
    for t in range(155):
        if log[t]['dog']!=1:continue
        ps,hs,wall=hist[t]
        built=sum(B[idx(w)] for w in wall)
        if built&fence!=fence or built&B[gate]:continue
        out['route_phase_turns']+=1
        pbits=sum({B[idx(p)] for p in ps});hbits=sum({B[idx(h)] for h in hs})
        after_people=sum({B[idx(h)] for h in hist[t+1][1]})
        blocked=pbits|wave(pbits)|hbits
        qualified.append((t,ps,tuple(map(idx,hs)),built,pbits,hbits,blocked,after_people,log[t]['cmd']))
    gate_eventually_walled=any(xy(gate) in w for _,_,w in hist)
    for body,shell,reuse in candidates:
        need=shell&~fence
        if need.bit_count()!=2:continue
        a,b=cells(need)
        agenda={a:[],b:[]}
        for t,ps,hs,built,pp,hh,blocked,hn,cmd in qualified:
            if built&(need|body) or hh&(body|need):continue
            for g in (a,b):
                wh=[i for i,h in enumerate(hs) if B[h]&wave(B[g])&~(body|shell)]
                if not wh:continue
                out['geometry_stance_visits']+=1
                if blocked&B[g]:continue
                wh=[i for i in wh if not cmd[i].islower()]
                if not wh:continue
                out['ready_legal_single_jobs']+=1
                idle=any(cmd[i]=='.' for i in wh)
                out['legal_single_idle']+=idle
                payload=tuple(i for i,p in enumerate(ps) if body&B[idx(p)])
                if payload:out['finish_gate_pet_windows']+=1
                agenda[g].append((t,idle,payload,pp))
        for g,h in ((a,b),(b,a)):
            for t,idle,_,_ in agenda[g]:
                for s,second_idle,pets,pp in agenda[h]:
                    if s<=t or s>t+8 or not pets:continue
                    out['paired_by8']+=1
                    # Discard if any logged trajectory would cross the new
                    # first wall before the proposed second action.
                    if any(steps[k]&B[g] or any(idx(p)==g for p in hist[k+1][1])
                           for k in range(t,s)):continue
                    out['paired_with_no_traced_crossing']+=1
                    out['paired_idle_both']+=idle and second_idle
                    out['paired_no_other_pet_in_large_corner']+=not pp&parent&~body
                    out['paired_never_ordinary_gate']+=not gate_eventually_walled
                    witness.append(dict(first=t,second=s,first_wall=xy(g),second_wall=xy(h),
                                        pen=[xy(z) for z in cells(body)],species=[kinds[i] for i in pets],
                                        both_idle=bool(idle and second_idle),
                                        other_in_parent=bool(pp&parent&~body),parent_gate_later=gate_eventually_walled))
    return out,witness

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','instrumented','cases','frozen','witness'):p.add_argument(n,type=Path)
    p.add_argument('--routes',action='store_true')
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    key={r['case']:r for r in csv.DictReader(a.frozen.open()) if r['model']=='control'}
    geometry=list(allshapes())
    tester=a.tester.resolve();exe=a.instrumented.resolve()
    out=csv.writer(sys.stdout,lineterminator='\n');out.writerow(['case','dog_phase',*(ROUTES if a.routes else FIELDS)])
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
        c,w=examine(hist,log,kinds,geometry,txt,a.routes)
        if w:wl[f.name]=w
        out.writerow([f.name,int(bool(log[0]['dog_walls'])),*[int(c[x]) for x in (ROUTES if a.routes else FIELDS)]]);sys.stdout.flush()
    a.witness.write_text(json.dumps(wl,indent=2)+'\n')
if __name__=='__main__':main()
