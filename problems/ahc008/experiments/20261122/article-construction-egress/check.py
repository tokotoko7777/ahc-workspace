"""Possible earlier ready-aisle egress after the same last paid brace wall."""
import argparse,csv,hashlib,json,sys,tempfile,collections,statistics
from pathlib import Path
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import ALL,B,ADJ,idx,flood,wave
from parallel_banks_anchored_cut_audit import buf,bits

def work_case(name,log,bridge,geo,history,cmds):
    from bridge import parse_geometry,nav,recover
    _,_,tube=parse_geometry(geo)
    s=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in history]
    nb=max(x.bank for x in tube)+1
    wall_by_bank=[buf(j for x in tube if x.bank==i for j in bits(x.wall)) for i in range(nb)]
    braces=buf(j for m in wall_by_bank for j in bits(m))
    ready=[]
    for t,(p,h,w) in enumerate(s):
        ready.append([x for x in tube if x.wall&w==x.wall])
    paid=collections.defaultdict(set)
    seals=collections.defaultdict(list)
    for row in log.read_text().splitlines():
        mode,t,b,a,i,*_=row.split()
        t,b,a,i=map(int,(t,b,a,i))
        paid[t]|={a,i}
        if mode=='I':
            assert s[t+1][2]&tube[b].gates==tube[b].gates
            seals[t].append(b)
    for row in bridge.read_text().splitlines():
        t,k,i,*_=map(int,row.split())
        if k in (3,4):paid[t].add(i)
    m=len(s[0][1])
    bands=[]
    for i in range(nb):
        T5=next((t for t in range(300) if sum(x.bank==i for x in ready[t])>=5),None)
        Tstaff=None;Tlock=None
        for t in range(300):
            if Tlock is None and any(tube[q].bank==i for q in seals[t]):Tlock=t+1
            if T5 is not None and t>=T5 and Tstaff is None:
                p,h,w=s[t]
                if any(x.bank==i and not (x.gates|x.body)&w and x.l in h and x.r in h and not x.body&buf(h)
                       for x in ready[t]):Tstaff=t
        bands.append(dict(case=name,bank=i,ready5=T5,staff=Tstaff,lock=Tlock))
    rows=[]
    opens=lambda t:ALL^s[t][2]
    def walljob(t,i):
        h=s[t][1][i]
        c=cmds[t][i]
        if c.islower():return idx(step(divmod(h,30),STEPS[c.upper()]))
        return None
    for i in range(m):
        builds=[(t,q) for t in range(133) if (q:=walljob(t,i)) is not None and B[q]&braces]
        if not builds:continue
        t,q=builds[-1]
        intact=s[t][1][i]
        # Preserve all actual paid tasks and their physical positions before
        # the final brace. Only the intervening ordinary approach can change.
        start=max((u for u in range(t) if i in paid[u] or cmds[u][i].islower()),default=-1)+1
        reach=B[s[start][1][i]]
        for u in range(start,t):
            added=s[u+1][2]&~s[u][2]
            reach&=~added
            reach=(reach|wave(reach))&opens(u+1)
        assert reach&B[intact]
        w=s[t+1][2]
        dead=w&~s[t][2]
        masks=[x.body for x in ready[t+1]]
        body=buf(j for a in masks for j in bits(a))
        open_=(ALL^w)&~body
        choices=wave(B[q])&reach&~dead&open_
        choices&=~B[intact]
        base_allowed=open_|(next((a for a in masks if B[intact]&a),0)&~w)
        base=nav(intact,base_allowed,30)
        candidates=[]
        for alt in bits(choices):
            paths=nav(alt,open_,10)
            for x in ready[t+1]:
                if (x.body|x.gates)&w:continue
                for g,other in ((x.l,x.r),(x.r,x.l)):
                    if g not in paths:continue
                    d=paths[g];old=base.get(g,99)
                    if d+2>old:continue
                    t0=t+1+d
                    if t0>=160:continue
                    path=recover(g,paths)[::-1]
                    stays_all_open=all(not B[p]&s[t+1+j][2] for j,p in enumerate(path))
                    if not stays_all_open:continue
                    earliest_paid=next((u for u in range(t+1,t0+1) if i in paid[u] or cmds[u][i].islower()),None)
                    p,h,wt=s[t0]
                    other_stance=(other in [a for j,a in enumerate(h) if j!=i])
                    shape_open=not (x.gates|x.body)&wt
                    after=list(h)
                    after[i]=g
                    pbits=buf(p)
                    exact=shape_open and other_stance and not x.body&buf(after) and not x.gates&(pbits|wave(pbits)|buf(after))
                    ids=[j for j,p0 in enumerate(p) if p0 in x.inside[1:3]] if exact else []
                    earlier=(bands[x.bank]['staff'] is None or t0<bands[x.bank]['staff'])
                    candidates.append(dict(tube=x.k,bank=x.bank,old_eta=old,new_eta=d,alt_stance=alt,goal=g,
                         approach_spare=t-start,early=t0,earlier_than_first_bank_staffing=earlier,
                         other_factual_stance=other_stance,factual_legal=ids,actual_paid_conflict=earliest_paid))
        candidates.sort(key=lambda x:(x['actual_paid_conflict'] is not None,
                                      -bool(x['factual_legal']),x['early'],x['new_eta']-x['old_eta']))
        rows.append(dict(case=name,worker=i,last_brace=t,wall=q,original_stance=intact,
                         possible_stance_count=choices.bit_count(),new_poses=candidates))
    return bands,rows

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','bridge_dir','tester','solver','cases','reference','bands','out','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path[:0]=[str(a.harness.resolve()),str(a.bridge_dir.resolve())]
    from benchmark_cases import run
    from benchmark import replay
    with open(a.reference) as f:refs=[r for r in csv.DictReader(f) if r['model']=='article']
    bands=[];records=[]
    for r in refs:
        name=r['case'];data=(a.cases/name).read_bytes();test=a.tester.resolve()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);l=d/'log';b=d/'bridge';g=d/'geo';tt=d/'trace';act=d/'act'
            o,e,rc,_=run([test,'env',f'AHC008_PAIR_COMMIT_LOG={l}',f'AHC008_BANK_LOG={g}',f'AHC008_BRIDGE_LOG={b}',a.solver.resolve()],data,3.)
            assert not rc and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tt],data,10.)
            assert (po,pe,pr)==(o,e,0)
            lines=tt.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(data.decode(),lines,cmds,detailed=True)[0]==int(r['score'])
            _,hist,_=observed(data,lines)
            x,y=work_case(name,l,b,g,hist,cmds);bands+=x;records+=y
        print(name,flush=True)
    v=[r for r in bands if r['ready5'] is not None]
    staff=[r['staff']-r['ready5'] for r in v if r['staff'] is not None]
    lock=[r['lock']-r['ready5'] for r in v if r['lock'] is not None]
    flat=[(r['case'],r['worker'],x) for r in records for x in r['new_poses']]
    ok=[r for r in flat if r[2]['actual_paid_conflict'] is None]
    present=[r for r in ok if r[2]['other_factual_stance']]
    legal=[r for r in present if r[2]['factual_legal']]
    certain=[r for r in present if r[2]['earlier_than_first_bank_staffing']]
    by=lambda x:dict(poses=len(x),cases=len({r[0] for r in x}),workers=len({r[:2] for r in x}))
    z=dict(verified=len(refs),banks=len(bands),ready5=len(v),ever_staffed=len(staff),ever_exact4=len(lock),
           ready5_median=statistics.median(r['ready5'] for r in v),
           after_ready_staff_median=statistics.median(staff),after_ready_capture_median=statistics.median(lock),
           actual_brace_workers=len(records),workers_reachable_reposition=sum(r['possible_stance_count']>0 for r in records),
           alternate_deltas=by(flat),preserves_paid=by(ok),other_guard_factual=by(present),
           prior_to_actual_first_staff=by(certain),actual_legal_occupied=by(legal))
    for p,d in [(a.bands,bands),(a.out,records)]:p.write_text(''.join(json.dumps(x,separators=(',',':'))+'\n' for x in d))
    a.summary.write_text(json.dumps(z,indent=2)+'\n');print(json.dumps(z,indent=2))
if __name__=='__main__':main()
