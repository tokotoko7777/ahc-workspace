"""Audit same-initial-tier shared irregular alternatives after actual v7 aborts."""
import argparse,csv,hashlib,json,sys,tempfile,collections,importlib.util
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,flood,wave
from parallel_banks_anchored_cut_audit import bits,buf,rect

def component(proj_wall,old,pet,hum):
    if (old|proj_wall)&buf(hum):return None
    op=ALL^(old|proj_wall)
    keeper=flood(B[hum[0]],op)
    if buf(hum)&keeper!=buf(hum) or proj_wall&B[pet] or B[pet]&keeper:return None
    pen=flood(B[pet],op)
    area=pen.bit_count()
    if not 4<=area<=48 or rect(pen):return None
    return keeper,pen

def run_case(name,log,history,match):
    s=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in history]
    F=flood(buf(s[-1][1]),ALL^s[-1][2])
    final={i for i,p in enumerate(s[-1][0]) if B[p]&F}
    generated=collections.defaultdict(set)
    search=collections.Counter()
    plans={}
    out=[]
    for ln in log.read_text().splitlines():
        kind,t,a,tier,*coords=map(int,ln.split())
        mask=buf(coords)
        if kind==0:search[t,tier]+=1;continue
        if kind==1:generated[t,tier].add(mask);continue
        if kind==2:
            plans[a]=mask,tier
            assert a==t and mask in generated[t,tier]
            continue
        if kind not in (4,5,6):continue
        out.append(dict(case=name,abort=t,launch=a,tier=tier,reason=kind,final_count=len(final),
                        finalized=(t>280 or not(1<=len(final)<=2))))
        rr=out[-1]
        if rr['finalized']:continue
        old,t0tier=plans[a]
        assert t0tier==tier and old==mask
        p0,h0,w0=s[a];pp,hh,ww=s[t]
        crew=buf(hh)
        keep0=flood(B[h0[0]],ALL^(w0|old))
        live0=flood(B[h0[0]],ALL^w0)
        live=flood(B[hh[0]],ALL^ww)
        keep=flood(B[hh[0]],ALL^(ww|old))
        captured0=[i for i,p in enumerate(p0) if B[p]&live0 and not B[p]&keep0]
        escaped=[i for i in captured0 if B[pp[i]]&live and (B[pp[i]]&(keep|old))]
        installed=old&ww&~w0
        rr.update(original_targets=captured0,escaped_targets=escaped,installed=list(bits(installed)),
                  original_pool=len(generated[a,tier]),alts=[],candidates=[])
        if not installed or not escaped or crew&live!=crew:continue
        for alt in sorted(generated[a,tier]):
            if alt==old:continue
            reuse=alt&installed
            if not reuse:continue
            rr['alts'].append(list(bits(alt)))
            for i in escaped:
                original=component(alt,w0,p0[i],h0)
                if not original:continue
                geo=component(alt,ww,pp[i],hh)
                if not geo:continue
                keep,pen=geo
                pending=alt&~ww
                if not pending:continue
                dct=dict(cut=list(bits(alt)),pet=i,original_area=original[1].bit_count(),
                         area=pen.bit_count(),final_missed=i in final,reused=list(bits(reuse)))
                rr['candidates'].append(dct)
                m=match(hh,list(bits(pending)),keep)
                if m is None:continue
                d,paths=m
                dct.update(eta=d,paths=paths)
                if d>6:continue
                pb=buf(pp)
                dct['now']=d==0 and not pending&(pb|wave(pb)|buf(hh))
                dct['factual']=-1
                if any(B[q]&s[t+f][2] for i0,path in paths for f,q in enumerate(path)):continue
                for e in range(t+max(1,d),min(t+6,299)+1):
                    pe,he,we=s[e]
                    if pending&we:continue
                    if any(B[path[-1]]&s[f][2] for i0,path in paths for f in range(t+len(path),e+1)):continue
                    prepared=he.copy()
                    for actor,path in paths:prepared[actor]=path[-1]
                    b=buf(pe)
                    if pending&(b|wave(b)|buf(prepared)):continue
                    if component(alt,we,pe[i],prepared):
                        dct['factual']=e
                        break
    return out,dict(searches=len(search),protected=sum(tier==1 for _,tier in search),
                    protection_empty=sum(tier==1 and not generated[t,tier] for t,tier in search),
                    fallback=sum(tier==2 for _,tier in search),attempts=len(plans),
                    one_shape=sum(len(generated[t,tier])==1 for t,(_,tier) in plans.items()))

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','wide_audit','tester','solver','cases','reference','out','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    spec=importlib.util.spec_from_file_location('wide_audit',a.wide_audit.resolve())
    wide=importlib.util.module_from_spec(spec);spec.loader.exec_module(wide)
    with open(a.reference) as f: refs=[r for r in csv.DictReader(f) if r['model']=='article']
    rows=[];stats=collections.Counter()
    for r in refs:
        name=r['case'];c=(a.cases/name).read_bytes();tt=a.tester.resolve()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);lt=d/'log';tr=d/'tape';act=d/'act'
            o,e,rc,_=run([tt,'env',f'AHC008_RETRY_AUDIT={lt}',a.solver.resolve()],c,3.)
            assert not rc and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([tt,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tr],c,10.)
            assert (po,pe,pr)==(o,e,0)
            lines=tr.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(c.decode(),lines,cmds,detailed=True)[0]==int(r['score'])
            _,history,_=observed(c,lines)
            x,q=run_case(name,lt,history,wide.match);rows+=x;stats.update(q)
        print(name,flush=True)
    tt=[x for x in rows if not x['finalized']]
    cc=[(x['case'],i) for x in tt for i in x['candidates']]
    short=[(n,q) for n,q in cc if q.get('eta',99)<=6]
    active=[(n,q) for n,q in short if q['final_missed']]
    cases=lambda q:len({p[0] for p in q})
    z=dict(verified=len(refs),search=stats,abort=len(rows),focussed=len(tt),focussed_cases=len({q['case'] for q in tt}),
           escaped=sum(bool(x['escaped_targets']) for x in tt),reuse_possible=sum(bool(x['alts']) for x in tt),
           geometry=len(cc),short=len(short),short_cases=cases(short),short_final_missed_cases=cases(active),
           immediate_final_missed_cases=cases([r for r in active if r[1]['now']]),
           factual_final_missed_cases=cases([r for r in active if r[1]['factual']>=0]))
    a.out.write_text(''.join(json.dumps(x,separators=(',',':'))+'\n' for x in rows))
    a.summary.write_text(json.dumps(z,indent=2)+'\n');print(json.dumps(z,indent=2))
if __name__=='__main__':main()
