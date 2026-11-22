"""Exact whole-case-clean geometry/dispatch audit on v7 old dev30."""
import argparse,csv,hashlib,json,sys,tempfile,collections
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ADJ,ALL,B,idx,flood,wave
from parallel_banks_anchored_cut_audit import bits,buf,distances

SHORT=6
def match(hh,ww,keep):
    if len(ww)>len(hh):return None
    dd=[distances(B[p],keep) for p in hh]
    options=[]
    for q in ww:
        y=[]
        for i,d in enumerate(dd):
            cand=[(d[x],x) for x in ADJ[q] if d[x]>=0]
            if cand:
                t,pos=min(cand)
                y.append((i,t,pos))
        if not y:return None
        options.append(y)
    dp={0:(0,[])}
    for j,op in enumerate(options):
        nxt={}
        for u,(m,p) in dp.items():
            for i,t,x in op:
                if u>>i&1:continue
                v=u|(1<<i);g=max(m,t)
                if v not in nxt or g<nxt[v][0]:nxt[v]=(g,p+[(i,t,x)])
        if not nxt:return None
        dp=nxt
    cost,ordered=min(dp.values())
    plans=[]
    for i,d,x in ordered:
        way=[x]
        while dd[i][way[-1]]:
            p=way[-1]
            way.append(min(j for j in ADJ[p] if dd[i][j]==dd[i][p]-1))
        plans.append((i,way[::-1]))
    return cost,plans

def geometry(cover,hh,pp,op,wall):
    live=flood(B[hh[0]],op)
    crew=buf(hh)
    if crew&live!=crew:return 'crew_split_before',None
    p=buf(pp)
    if not live&p:return 'already_clear',None
    if wall&p:return 'pet_on_proposed_cut',None
    keep=flood(B[hh[0]],op&~wall)
    if crew&keep!=crew:return 'crew_not_retained',None
    if p&keep:return 'not_full_case',None
    if keep.bit_count()<300 or 10*keep.bit_count()<7*live.bit_count():return 'below_area',None
    return 'whole_geom',(live,keep)

def evaluate(t,mask,snap):
    hh,pp,wall=snap[t]
    o=ALL^wall
    why,geo=geometry(None,hh,pp,o,mask)
    item=dict(status=why)
    if not geo:return item
    before,keep=geo
    jobs=list(bits(mask))
    item.update(retained=keep.bit_count(),root_area=before.bit_count(),
                 target=[i for i,p in enumerate(pp) if B[p]&before])
    blocked=buf(pp);blocked|=wave(blocked)|buf(hh)
    packed=match(hh,jobs,keep)
    if packed is None:
        return dict(item,status='no_one_job_per_person')
    d,paths=packed
    immediate=d==0 and not mask&blocked
    item.update(eta=d,now_legal=not bool(mask&blocked),immediate=immediate)
    if immediate:
        item['status']='now'
        item['crew']=paths
        return item
    if d>SHORT:
        item['status']='too_far'
        return item
    item['status']='short'
    item['crew']=paths
    item['observed_window']=-1
    if any(B[p]&snap[t+s][2] for i,path in paths for s,p in enumerate(path)):return item
    for u in range(t+max(1,d),min(t+SHORT,299)+1):
        hu,pu,wu=snap[u]
        if mask&wu:continue
        if any(B[path[-1]]&snap[f][2] for i,path in paths for f in range(t+len(path),u+1)):continue
        proposed=hu.copy()
        for i,path in paths:proposed[i]=path[-1]
        at=buf(pu);forbidden=at|wave(at)|buf(proposed)
        if forbidden&mask:continue
        k,g=geometry(None,proposed,pu,ALL^wu,mask)
        if g:
            item['observed_window']=u
            break
    return item

def check(log,history,name,clear_turn):
    events=collections.defaultdict(dict)
    for r in log.read_text().splitlines():
        t,k,*g=map(int,r.split())
        ev=events[t];k={0:'searched',1:'candidates',2:'chosen'}[k]
        if k=='candidates':ev.setdefault(k,[]).append(buf(g))
        else:assert k not in ev;ev[k]=buf(g) if g else 0
    snap=[(list(map(idx,h)),list(map(idx,p)),buf(map(idx,w))) for p,h,w in history]
    rows=[];counts=collections.Counter()
    for t,v in sorted(events.items()):
        searched='searched' in v
        counts['search_turns' if searched else 'continued_turns']+=1
        mask=v.get('chosen')
        if mask is not None:
            ch=evaluate(t,mask,snap)
        else:ch={'status':'no_chosen'}
        for alt in set(v.get('candidates',[])):
            assert searched
            counts['qualified_proxy']+=1
            r=evaluate(t,alt,snap)
            qual=r['status'] in ('now','short')
            r.update(case=name,turn=t,walls=list(bits(alt)),chosen=list(bits(mask)) if mask else [],
                     is_chosen=alt==mask,chosen_status=ch['status'],chosen_observed_window=ch.get('observed_window'),
                     ignored_immediate=(r['status']=='now' and alt!=mask and ch['status']!='now'),
                     ignored_short=(r['status']=='short' and qual and alt!=mask and
                                    ch['status'] not in ('now','short')),
                     original_final_allclear=clear_turn>=0)
            rows.append(r)
        if 'chosen' in v:counts['chosen_'+ch['status']]+=1
    return rows,counts

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','tester','solver','cases','reference','events','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with open(a.reference) as f:refs=[r for r in csv.DictReader(f) if r['model']=='article']
    rows=[];count=collections.Counter()
    for r in refs:
        name=r['case'];data=(a.cases/name).read_bytes()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);log=d/'log';act=d/'actions';trace=d/'trace';t=a.tester.resolve()
            o,e,rc,_=run([t,'env',f'AHC008_WIDE_CLEAR_LOG={log}',a.solver.resolve()],data,3.)
            assert rc==0 and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([t,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                             '--playback',act,trace],data,10.)
            assert (po,pe,pr)==(o,e,0)
            tape=trace.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(data.decode(),tape,cmds,detailed=True)[0]==int(r['score'])
            _,hist,_=observed(data,tape)
            e,c=check(log,hist,name,int(r['first_all_clear']))
            rows+=e;count.update(c)
        print(name,flush=True)
    uncaught=[r for r in rows if not r['original_final_allclear']]
    cases=lambda rows:len({r['case'] for r in rows})
    s=dict(verified=len(refs),counts=count,candidates=dict(collections.Counter(r['status'] for r in rows)),
           candidate_cases=cases(rows),
           overlooked_immediate=sum(r['ignored_immediate'] for r in rows),
           overlooked_immediate_unresolved=cases([r for r in uncaught if r['ignored_immediate']]),
           overlooked_short=sum(r['ignored_short'] for r in rows),
           overlooked_short_unresolved=cases([r for r in uncaught if r['ignored_short']]),
           overlooked_short_factual_unresolved=cases(
               [r for r in uncaught if r['ignored_short'] and r['observed_window']>=0]))
    a.events.write_text(''.join(json.dumps(x,separators=(',',':'))+'\n' for x in rows))
    a.summary.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))
if __name__=='__main__':main()
