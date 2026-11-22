"""Observed candidates for a single early door in staged exact-four tube banks."""
import argparse,csv,hashlib,itertools,sys,tempfile
from collections import Counter
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,wave,flood

def examine(hist,kinds,meta):
    _,_,stop=map(int,meta.splitlines()[0].split())
    tubes=[]
    for line in meta.splitlines()[1:]:
        bank,row,l,g,j,r,*z=map(int,line.split())
        inn=sum(B[x] for x in z[:4]);side=sum(B[x] for x in z[4:])
        tubes.append((l,g,j,r,inn,side,B[z[1]]|B[z[2]]))
    credited={}
    for t in range(stop):
        ps,hs,ws=hist[t];_,_,qn=hist[t+1]
        built=sum(B[idx(w)] for w in ws);new=sum(B[idx(w)] for w in qn-ws)
        for l,g,j,r,body,side,mid in tubes:
            if new&(B[g]|B[j])!=B[g]|B[j] or side&built!=side:continue
            for i,p in enumerate(ps):
                if body&B[idx(p)]:credited[i]=t
    final_ps,final_hs,final_ws=hist[-1]
    finalpass=ALL^sum(B[idx(w)] for w in final_ws)
    coherent=flood(sum({B[idx(h)] for h in final_hs}),finalpass)
    candidates={}
    for t in range(stop):
        ps,hs,ws=hist[t]
        humans=list(map(idx,hs));pets=list(map(idx,ps))
        built=sum(B[idx(w)] for w in ws)
        hb=sum({B[h] for h in humans});pb=sum({B[p] for p in pets})
        ban=hb|pb|wave(pb)
        for ti,(l,g,j,r,body,side,mid) in enumerate(tubes):
            if side&built!=side or (B[g]|B[j])&built or body&hb:continue
            target=[i for i,p in enumerate(pets) if kinds[i]<=3 and B[p]&mid]
            if not target:continue
            for left,near,far,right in ((l,g,j,r),(r,j,g,l)):
                if not B[left]&hb or B[near]&ban:continue
                # If normal simultaneous seal already executable, it wins.
                if B[right]&hb and not B[far]&ban:continue
                grid=ALL&~(built|body|B[near]|B[far])
                reach=B[right];choice=-1;eta=999
                for d in range(4):
                    who=[i for i,h in enumerate(humans) if h!=left and reach&B[h]]
                    if who:choice=who[0];eta=d;break
                    reach|=wave(reach)&grid
                if choice<0:continue
                for a in target:
                    k=(t,ti,a)
                    row=(eta,near,far,int(bool(B[far]&ban)),int(a in credited),
                         credited.get(a,-1),int(bool(B[idx(final_ps[a])]&coherent)),kinds[a])
                    if k not in candidates or row[0]<candidates[k][0]:candidates[k]=row
    return candidates

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','solver','cases','staged'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    key={r['case']:r for r in csv.DictReader(a.staged.open()) if r['model']=='staged'}
    tester=a.tester.resolve();exe=a.solver.resolve()
    out=csv.writer(sys.stdout,lineterminator='\n')
    out.writerow(['case','turn','tube','pet','second_eta','near_door','far_door','far_blocked_now','baseline_banked',
                  'baseline_seal_turn','baseline_final_reachable','species'])
    for f in sorted(a.cases.glob('*.txt')):
        case=f.read_bytes();row=key[f.name]
        with tempfile.TemporaryDirectory() as td:
            meta=Path(td)/'meta'
            act,err,rc,_=run([tester,'env',f'AHC008_BANK_LOG={meta}',exe,'3'],case,3.)
            assert rc==0 and hashlib.sha256(act).hexdigest()==row['actions_sha256']
            assert int(err.split()[-1])==int(row['score'])
            p=Path(td)/'a';p.write_bytes(act);tr=Path(td)/'trace'
            a2,e2,r2,_=run([tester,sys.executable,a.harness.resolve()/'benchmark_cases.py','--playback',p,tr],case,10.)
            assert (a2,e2,r2)==(act,err,0)
            txt=tr.read_text().splitlines()
            score,*_=replay(case.decode(),txt,act.decode().splitlines(),detailed=True)
            assert score==int(row['score'])
            kinds,hist,_=observed(case,txt)
            out.writerows((f.name,*k,*v) for k,v in examine(hist,kinds,meta.read_text()).items());sys.stdout.flush()
if __name__=='__main__':main()
