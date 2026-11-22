"""One-step staffed setup then whole-case legal full clear, factual ceiling only."""
import argparse,csv,hashlib,itertools,json,sys,tempfile,collections
from pathlib import Path
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import ALL,B,idx,flood,wave
from parallel_banks_anchored_cut_audit import buf,bits,rect

def search(t,s,cmds):
    p,h,w=s[t];p1,h1,w1=s[t+1];_,h2,w2=s[t+2]
    root=flood(B[h[0]],ALL^w);cur=root.bit_count()
    demand=max(300,(65*cur+99)//100)
    assert cur>=300 and buf(h)&root==buf(h)
    forb=buf(p1);forb|=wave(forb)
    def walls(a,hum):
        z={}
        for i,c in enumerate(cmds[a]):
            if c.islower():z[i]=idx(step(divmod(hum[i],30),STEPS[c.upper()]))
        return z
    oldmove=walls(t,h);oldfinish=walls(t+1,h1)
    fixed=buf(oldfinish.values())
    opts=collections.defaultdict(list)
    for i,p0 in enumerate(h):
        if i in oldmove:continue # preserve every prior paid physical wall
        poses=(B[p0]|wave(B[p0]))&(ALL^w1)&~fixed
        for q in bits(poses):
            for z in bits(wave(B[q])&root&~(forb|w1)):
                if i in oldfinish and oldfinish[i]!=z:continue
                opts[z].append((i,q))
    opts=dict(opts)
    witnesses=[];tested=0;geom=0
    for k in (1,2):
        for walls_ in itertools.combinations(sorted(opts),k):
            m=buf(walls_)
            if m&~fixed==0:continue # no new physical wall
            tested+=1
            board=w1|fixed|m
            op=root&~board
            keeper=0;qq=op
            while qq.bit_count()>=demand:
                v=flood(qq&-qq,op)
                if v.bit_count()>=demand:
                    keeper=v;break
                qq&=~v
            if not keeper or keeper&buf(p1):continue
            geom+=1
            record=None
            for a in itertools.product(*(opts[z] for z in walls_)):
                if k==2 and a[0][0]==a[1][0]:continue
                if not any(h[i]!=q for i,q in a):continue
                # Preserve all other actual movements and physical walls on both phases.
                ha=list(h1);hb=list(h2)
                good=True
                for z,(i,q) in zip(walls_,a,strict=True):
                    ha[i]=hb[i]=q
                if any(B[x]&board for x in ha+hb):continue
                if buf(ha)&keeper!=buf(ha) or buf(hb)&keeper!=buf(hb):continue
                # If the old actor already owns a repeated physical wall,
                # only it may be considered its paired builder.
                for i,z in oldfinish.items():
                    if z in walls_ and (i,ha[i]) not in a:good=False;break
                if not good:continue
                pen={}
                for j,p in enumerate(p1):
                    if not B[p]&root:continue
                    c=flood(B[p],ALL^board)
                    assert not c&keeper
                    q=pen.setdefault(c&-c,dict(area=c.bit_count(),nonrect=not rect(c),animals=[]))
                    q['animals'].append(j)
                row=dict(walls=walls_,setup=a,keep=keeper.bit_count(),pens=list(pen.values()),
                         already_actual_setup=sum(h1[i]==q for i,q in a))
                if record is None or row['already_actual_setup']>record['already_actual_setup']:record=row
            if record:witnesses.append(record)
    return dict(turn=t,current_area=cur,sets=tested,geometric=geom,positive=len(witnesses),
             best=max(witnesses,key=lambda x:(x['keep'],x['already_actual_setup'])) if witnesses else None)

def selfcheck():
    # Guard at (0,4) must move to (0,3) before placing top-left pen gate.
    p=[30];h=[4,90,99,180,181]
    pen={0,1,30,31};g=2
    w=buf(c for x in pen for c in bits(wave(B[x])) if c not in pen and c!=g)
    s=[(p,h,w)]*3
    x=search(0,s,['.....']*2)
    assert x['best'] and x['best']['walls']==(2,) and x['best']['setup']==((0,3),)
    p=[1];s=[(p,h,w)]*3
    x=search(0,s,['.....']*2);assert not x['best'] or 2 not in x['best']['walls']

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','tester','solver','cases','reference','immediate','out','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    selfcheck()
    with open(a.reference) as f:ref={r['case']:r for r in csv.DictReader(f) if r['model']=='article'}
    by=collections.defaultdict(list)
    with open(a.immediate) as f:
        for q in map(json.loads,f):
            if q['turn']<299:by[q['case']].append(q['turn'])
    rows=[]
    for name,ts in sorted(by.items()):
        r=ref[name];data=(a.cases/name).read_bytes();tool=a.tester.resolve()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);tr=d/'tape';act=d/'actions'
            o,e,rc,_=run([tool,a.solver.resolve()],data,3.)
            assert not rc and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([tool,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tr],data,10.)
            assert (po,pe,pr)==(o,e,0)
            tape=tr.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(data.decode(),tape,cmds,detailed=True)[0]==int(r['score'])
            _,history,_=observed(data,tape)
            s=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in history]
            mm={int(x.split()[0]):int(x.split()[1]) for x in r['telemetry'].split(';') if x}
            for t in ts:
                q=search(t,s,cmds);q.update(case=name,baseline_mode=[mm.get(t),mm.get(t+1)])
                rows.append(q)
        print(name,flush=True)
    good=[q for q in rows if q['best']]
    z=dict(verified=len(by),states=len(rows),tested=sum(x['sets'] for x in rows),
           geometric=sum(x['geometric'] for x in rows),positive=sum(x['positive'] for x in rows),
           positive_turns=len(good),positive_cases=len({q['case'] for q in good}),
           original_modes=dict(collections.Counter(str(q['baseline_mode']) for q in good)))
    a.out.write_text(''.join(json.dumps(q,separators=(',',':'))+'\n' for q in rows))
    a.summary.write_text(json.dumps(z,indent=2)+'\n');print(json.dumps(z,indent=2))
if __name__=='__main__':main()
