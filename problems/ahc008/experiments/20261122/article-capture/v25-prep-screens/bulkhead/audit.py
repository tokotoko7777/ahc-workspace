"""Read-only screen: actual v25 histories; projected work is not a rollout."""
from pathlib import Path
from collections import Counter
import csv,hashlib,json,sys,tempfile
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
V=R/'solve'
C=W/'ahc008_v24_five_retention/m5-dev30/in'
D={'U':-30,'D':30,'L':-1,'R':1,'u':-30,'d':30,'l':-1,'r':1}
START,END=175,215
def inspect(raw,moves,act,geom):
    kinds,hist,_=observed(raw,moves)
    st=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in hist]
    rot,shift,by_tube=parse_geometry(geom)
    pos=lambda r,c:c*30+r if rot else r*30+c
    cols=[shift+7*i for i in range(3)]
    # The entire primary aisle set must remain in one human component
    # until closure; other originally viable rail stances must also survive.
    roads=buf(pos(r,c) for r in range(30) for c in cols)
    gaps=[]; eligible=[]; witnesses=[]
    for rr in (6,24):
        doors=[pos(rr,c) for c in cols]
        dm=buf(doors)
        brace=buf(pos(rr,c+7*k) for k in range(2) for c in range(shift+2,shift+6))
        prep=buf(pos(rr,c) for c in range(30))&~(dm|brace)
        for start in range(START,END+1):
            p,h,w=st[start]
            missing=(prep&~w).bit_count()
            gaps.append([rr,start,missing,(brace&~w).bit_count(),(dm&w).bit_count()])
            if missing>8 or dm&w:continue
            eligible.append((rr,start))
            # A conservative sequential witness: at most one added wall
            # per turn by an already adjacent dot worker, no changed moves.
            # Dot may have been paid hold, so it is an optimistic idle ceiling.
            # Choice is deterministic (lowest wall then lowest actor).
            added=0; jobs=[]
            for t in range(start,END+1):
                p,h,w=st[t]; occ=buf(p)|buf(h)
                if (added&occ) or w&dm:break
                old=flood(buf(h),ALL^w)
                live=buf(q for x in by_tube for q in (x.l,x.r) if not w&(x.gates|x.body))
                # Respect every actual wall/move in this turn.
                actualW=buf(h[i]+D[c] for i,c in enumerate(act[t]) if c.islower())
                actualDest=buf(h[i]+D[c] if c.isupper() else h[i] for i,c in enumerate(act[t]))
                if added&actualDest:break
                projected=w|added|actualW
                outside=flood(B[h[0]],ALL^projected)
                if (roads|live|buf(h)|actualDest)&~outside:break
                blocker=occ|wave(buf(p))|actualDest
                idle=[i for i,c in enumerate(act[t]) if c=='.']
                if (brace|prep)&~projected==0 and not (dm&(blocker|actualW)):
                    finalwall=projected|dm
                    safe=flood(B[h[0]],ALL^finalwall)
                    if not (buf(h)|actualDest)&~safe and safe.bit_count()>=300 and 4*safe.bit_count()>=3*old.bit_count():
                        # Closers stand on retained side and do not displace
                        # non-dot factual actions. Every door needs a worker.
                        def rec(j,mask):
                            if j==len(doors):return []
                            for i in idle:
                                if not (mask>>i&1) and doors[j] in ADJ[h[i]] and safe&B[h[i]]:
                                    a=rec(j+1,mask|1<<i)
                                    if a is not None:return [i]+a
                            return None
                        staff=rec(0,0)
                        ids=[i for i,q in enumerate(p) if old&B[q] and not safe&B[q]]
                        if staff is not None and ids:
                            witnesses.append(dict(row=rr,start=start,t=t,prep=jobs,staff=staff,
                                retained=safe.bit_count(),current=old.bit_count(),pets=ids))
                            break
                if len(jobs)>=8:continue
                for q in bits(prep&~projected&~blocker):
                    actors=[i for i in idle if q in ADJ[h[i]] and outside&B[h[i]]]
                    if not actors:continue
                    after=flood(B[h[0]],ALL^(projected|B[q]))
                    if (roads|live|buf(h)|actualDest)&~after:continue
                    jobs.append([t,actors[0],q]);added|=B[q];break
    return dict(gaps=gaps,eligible=eligible,witnesses=witnesses)
def main():
    out=Path(sys.argv[1]);cases=sorted(C.glob('*.txt'))
    ref={r['case']:r for r in csv.DictReader((R/'dev30.csv').open()) if r['model']=='article'}
    assert len(cases)==len(ref)==30
    rows=[]
    for f in cases:
        raw=f.read_bytes(); old=ref[f.name]
        assert old['input_sha256']==hashlib.sha256(raw).hexdigest()
        with tempfile.TemporaryDirectory() as q:
            d=Path(q); cmd=d/'actions'; pets=d/'pets'; geo=d/'geo'
            a,e,rc,_=run([str(T),'env','AHC008_BANK_LOG='+str(geo),str(V)],raw,4.5)
            assert rc==0 and hashlib.sha256(a).hexdigest()==old['actions_sha256'],f.name
            cmd.write_bytes(a)
            b,err,rc,_=run([str(T),sys.executable,H/'benchmark_cases.py','--playback',cmd,pets],raw,12)
            assert (b,err,rc)==(a,e,0)
            act=a.decode().splitlines(); moves=pets.read_text().splitlines()
            score,*_=replay(raw.decode(),moves,act,detailed=True)
            assert score==int(old['score']) and e.decode().strip()==f'Score = {score}',f.name
            rows.append(dict(case=f.name,exact=True,score=score,**inspect(raw,moves,act,geo)))
        print(f.name,'min missing',min(v[2] for v in rows[-1]['gaps']),'eligible',len(rows[-1]['eligible']),'witnesses',len(rows[-1]['witnesses']),flush=True)
    out.write_text(json.dumps(rows,indent=2)+'\n')
    h=Counter((v[0],v[2]) for x in rows for v in x['gaps'])
    print('gap histogram',sorted(h.items()))
    print('eligible cases',sum(bool(x['eligible']) for x in rows),'witness cases',sum(bool(x['witnesses']) for x in rows))
if __name__=='__main__':main()
