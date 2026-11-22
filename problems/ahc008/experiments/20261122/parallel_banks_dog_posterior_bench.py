"""Frozen official trace audit of conditioned dog prediction, no solver action changes."""
import argparse,csv,hashlib,io,json,math,subprocess,sys,tempfile
from collections import defaultdict
from pathlib import Path
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import B,idx,wave

def frame_data(history, commands, lines, kinds, tubes,release):
    n,m=len(kinds),len(history[0][1]);s=io.StringIO()
    def put(xx):s.write(' '.join(map(str,xx))+'\n')
    put((n,m,len(tubes),release));put(kinds)
    for x in tubes:put([*x[2:10],len(x[10:]),*x[10:]])
    for t in range(300):
        ps,hs,ws=history[t]
        post=[step(p,STEPS[c]) if c in STEPS else p for p,c in zip(hs,commands[t])]
        walls=['0']*900
        for p in history[t+1][2]:walls[idx(p)]='1'
        put(map(idx,ps));put(map(idx,post));put([''.join(walls)])
        put(lines[2*t+1].split())
    return s.getvalue()

def outcomes(history,t,tube,p):
    _,_,l,g,k,r,*z=tube;body=set(z[:4]);side=set(z[4:])
    out=[]
    for h in (1,2,3):
        ps,hs,w=history[t+h]
        hpos=set(map(idx,hs));walls=set(map(idx,w));occ=set(map(idx,ps))
        target=idx(ps[p])
        ready=side<=walls and not ({g,k}|body)&walls and not body&hpos
        petmask=sum(B[j] for j in occ)
        legal=ready and target in (z[1],z[2]) and not (B[g]|B[k])&(petmask|wave(petmask)|sum(B[j] for j in hpos))
        out.append((int(ready and target in (z[1],z[2])),int(legal),int(legal and l in hpos and r in hpos)))
    return dict(safe1=out[0][0],safe3=max(z[0] for z in out),legal1=out[0][1],legal3=max(z[1] for z in out),
                staffed3=max(z[2] for z in out))

def compare(q, suffix, metric, target=None):
    subsets=defaultdict(list)
    for x in q:subsets[x['case']].append(x)
    models=('post','reset','heading')
    mm={m:[] for m in models}
    per=defaultdict(dict)
    for case,rows in subsets.items():
        for m in models:
            r=[(-math.log(max(1e-12,x[m+suffix])) if metric=='nll' else (x[m+suffix]-x[target])**2) for x in rows]
            mm[m].extend(r);per[case][m]=sum(r)/len(r)
    return dict(rows=len(q),cases=len(per),positives=None if target is None else sum(x[target] for x in q),
       metrics={m:sum(v)/len(v) if v else None for m,v in mm.items()},
       cases_post_better={m:sum(v['post']<v[m]-1e-9 for v in per.values()) for m in models[1:]},
       cases_post_worse={m:sum(v['post']>v[m]+1e-9 for v in per.values()) for m in models[1:]})

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','solver','probe','inputs','control','out','summary'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark import replay
    from benchmark_cases import run
    with a.control.open() as f:ref=[r for r in csv.DictReader(f) if r['model']=='narrow']
    out=[]
    for r in ref:
        name=r['case'];case=(a.inputs/name).read_bytes();release=int(r['release'])
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);meta=d/'geom';act=d/'act';trace=d/'trace'
            test=a.tester.resolve()
            stdout,e,rc,_=run([test,'env',f'AHC008_BANK_LOG={meta}',a.solver.resolve()],case,3.)
            assert rc==0 and e.strip()==('Score = '+r['score']).encode()
            assert r['actions_sha256']==hashlib.sha256(stdout).hexdigest()
            act.write_bytes(stdout)
            pb,e2,rc2,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py','--playback',act,trace],case,10.)
            assert (pb,e2,rc2)==(stdout,e,0)
            lines=trace.read_text().splitlines();commands=stdout.decode().splitlines()
            assert replay(case.decode(),lines,commands,detailed=True)[0]==int(r['score'])
            kinds,hist,_=observed(case,lines)
            tubes=[list(map(int,x.split())) for x in meta.read_text().splitlines()[1:]]
            probe=subprocess.run([str(a.probe.resolve())],text=True,input=frame_data(hist,commands,lines,kinds,tubes,release),
                                 capture_output=True,check=True)
            for line in probe.stdout.splitlines():
                k,t,i,*row=line.split();t,i=int(t),int(i)
                d=dict(case=name,kind=k,turn=t,pet=i,release=release)
                if k=='D':
                    d.update(zip(('post','reset','heading'),map(float,row)))
                else:
                    tube,eta=map(int,row[:2])
                    d.update(tube=tube,eta=eta,**outcomes(hist,t,tubes[tube],i))
                    d.update(zip(('post1','reset1','heading1','post3','reset3','heading3'),map(float,row[2:])))
                out.append(d)
        print(name,flush=True)
    a.out.write_text(''.join(json.dumps(x,separators=(',',':'))+'\n' for x in out))
    dr=[r for r in out if r['kind']=='D'];pr=[r for r in out if r['kind']=='P']
    summary=dict(verified_cases=len(ref),all_step=compare(dr,'','nll'),
       bank_step=compare([r for r in dr if 101<=r['turn']<r['release']],'','nll'))
    for name,q in [('all',pr),('post101',[r for r in pr if r['turn']>=101])]:
        summary[name+'_tube']={k:compare(q,str(1 if k.endswith('1') else 3),'brier',k)
                               for k in ('safe1','safe3','legal1','legal3','staffed3')}
    a.summary.write_text(json.dumps(summary,indent=2)+'\n');print(json.dumps(summary,indent=2))
if __name__=='__main__':main()
