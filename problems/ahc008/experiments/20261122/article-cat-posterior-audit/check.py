"""Evaluate diagnostic v7 cat forecasts without changing official output."""
import argparse,csv,hashlib,json,math,sys,tempfile,collections
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,flood,wave
from parallel_banks_anchored_cut_audit import buf

def metric(rows,key,weight=False):
    if not rows:return None
    n=len(rows)
    a=sum((x['old' if weight else 'old_safe']-x[key])**2 for x in rows)/n
    b=sum((x['new' if weight else 'new_safe']-x[key])**2 for x in rows)/n
    return [a,b]
def auc(xs,label,key):
    sorted_=sorted((r[key],int(r[label])) for r in xs)
    pos=neg=seen=0;ans=0.;i=0
    while i<len(xs):
        j=i+1
        while j<len(xs) and sorted_[j][0]==sorted_[i][0]:j+=1
        p=sum(e[1] for e in sorted_[i:j]);n=j-i-p
        ans+=p*(seen+n/2)
        seen+=n;pos+=p;neg+=n;i=j
    return ans/(pos*neg) if pos*neg else None
def prefer_group(rr,keys,lab):
    groups=collections.defaultdict(list)
    for r in rr:groups[tuple(r[k] for k in keys)].append(r)
    win=loss=tie=0;rank=[0.,0.];comparable=0
    for g in groups.values():
        a,b=metric(g,lab,True)
        win+=b<a;loss+=b>a;tie+=b==a
        if any(x[lab] for x in g) and not all(x[lab] for x in g):
            for j,k in enumerate(['old','new']):rank[j]+=auc(g,lab,k)
            comparable+=1
    return dict(win=win,loss=loss,tie=tie,comparable=comparable,
                rank=[x/comparable for x in rank] if comparable else [])
def run_probe(name,log,geometry,history,kinds):
    from bridge import parse_geometry
    _,_,tube=parse_geometry(geometry)
    s=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in history]
    final=flood(buf(s[-1][1]),ALL^s[-1][2]);missed=[i for i,p in enumerate(s[-1][0]) if B[p]&final]
    nll=[];rr=[];tim={}
    for l in log.read_text().splitlines():
        k,*r=l.split()
        if k=='T':
            assert not tim
            tim=dict(case=name,observe_seconds=float(r[0]),forecast_seconds=float(r[1]))
        if k=='N':
            t,j,d=map(int,r[:3]);p,q=map(float,r[3:])
            assert kinds[j]==5 and 0<=p<=1.0001 and 0<=q<=1.0001
            nll.append(dict(case=name,t=t,pet=j,dir=d,old=p,new=q))
        if k=='P':
            t,u,eta,p,a,b=map(int,r[:6]);x=tube[u]
            assert kinds[p]==5 and a!=b and 1<=eta<=3
            p0,q0,other=map(float,r[6:])
            assert 0<p0<=1.0001 and 0<=q0<=1.0001 and 0<=other<=1.0001
            aa,hh,w=s[t+eta]
            body=buf(x.inside);pet=buf(aa)
            assert x.wall&s[t][2]==x.wall and not (x.gates|body)&s[t][2]
            inside=aa[p] in x.inside[1:3]
            legal=inside and not (x.gates|body)&w and not x.gates&(pet|wave(pet)|buf(hh)) and not body&buf(hh)
            actual_stance=int(legal and x.l in hh and x.r in hh)
            matched_stance=int(legal and hh[a]==x.l and hh[b]==x.r)
            acted=int(legal and s[t+eta+1][2]&x.gates==x.gates)
            rr.append(dict(case=name,t=t,tube=u,eta=eta,pet=p,actors=[a,b],
                            old_safe=p0,new_safe=q0,others=other,old=p0*other,new=q0*other,
                            inside=int(inside),legal=int(legal),actual_stance=actual_stance,
                            matched_stance=matched_stance,acted=acted,otherwise_final_missed=int(p in missed)))
    return rr,nll,tim

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','bridge_dir','tester','solver','cases','reference','out','nll','timing','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path[:0]=[str(a.harness.resolve()),str(a.bridge_dir.resolve())]
    from benchmark_cases import run
    from benchmark import replay
    with open(a.reference) as f:refs=[r for r in csv.DictReader(f) if r['model']=='article']
    all_,steps,times=[],[],[]
    for r in refs:
        name=r['case'];c=(a.cases/name).read_bytes();test=a.tester.resolve()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);log=d/'log';geo=d/'geo';act=d/'act';tr=d/'tr'
            o,e,rc,elapsed=run([test,'env',f'AHC008_CAT_DIAG={log}',f'AHC008_BANK_LOG={geo}',a.solver.resolve()],c,3.)
            assert not rc and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tr],c,10.)
            assert (po,pe,pr)==(o,e,0)
            lines=tr.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(c.decode(),lines,cmds,detailed=True)[0]==int(r['score'])
            kinds,history,_=observed(c,lines)
            x,y,z=run_probe(name,log,geo,history,kinds)
            z['elapsed_with_diagnostic']=elapsed
            all_+=x;steps+=y;times.append(z)
        print(name,flush=True)
    eps=1e-9
    nnl=lambda xs,k:sum(-math.log(max(eps,r[k])) for r in xs)/len(xs)
    allnll=[nnl(steps,k) for k in ('old','new')]
    cat=lambda xs:len({(r['case'],r['pet']) for r in xs})
    cases=lambda xs:len({r['case'] for r in xs})
    s=dict(verified=len(refs),nll_rows=len(steps),nll=allnll,nll_case_count=cases(steps),nll_cat_count=cat(steps),
           rows=len(all_),cases=cases(all_),cats=cat(all_),
           brier_inside=metric(all_,'inside'),
           brier_legal=metric(all_,'legal',True),brier_staffed=metric(all_,'actual_stance',True),
           auc_legal=[auc(all_,'legal',k) for k in ('old','new')],
           shape_rank=prefer_group(all_,['case','t','pet','eta'],'legal'),
           case_cluster=prefer_group(all_,['case'],'legal'),
           pet_cluster=prefer_group(all_,['case','pet'],'legal'),
           max_diag_obs=max(x['observe_seconds'] for x in times),max_diag_forecast=max(x['forecast_seconds'] for x in times),
           max_diag_wall=max(x['elapsed_with_diagnostic'] for x in times),
           positive={},by_horizon={})
    for k in ('inside','legal','actual_stance','matched_stance','acted'):
        v=[x for x in all_ if x[k]]
        miss=[x for x in v if x['otherwise_final_missed']]
        s['positive'][k]=dict(rows=len(v),cases=cases(v),cats=cat(v),otherwise_final_missed=cat(miss),
                             otherwise_final_missed_cases=cases(miss))
    for h in range(1,4):
        vv=[z for z in all_ if z['eta']==h]
        s['by_horizon'][h]=dict(rows=len(vv),legal=sum(x['legal'] for x in vv),brier=metric(vv,'legal',True))
    for p,d in [(a.out,all_),(a.nll,steps),(a.timing,times)]:
        p.write_text(''.join(json.dumps(x,separators=(',',':'))+'\n' for x in d))
    a.summary.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))
if __name__=='__main__':main()
