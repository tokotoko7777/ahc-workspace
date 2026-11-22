"""Retrospective staged-bank late crossbar feasibility, no changed actions."""
import argparse,csv,hashlib,sys,tempfile
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ADJ,ALL,B,idx,wave,cells,flood

def dist(start,passable):
    d=[999]*900;d[start]=0;q=[start]
    for u in q:
        for v in ADJ[u]:
            if passable&B[v] and d[v]==999:d[v]=d[u]+1;q.append(v)
    return d

def lower(needs,ds,key,limit=30):
    requirement=[]
    for pos in needs:
        st=[q for q in ADJ[pos] if key&B[q]]
        if not st:return 999
        requirement.append([min(d[q] for q in st) for d in ds])
    escape=max(min(d[q] for q in cells(key)) for d in ds)
    l=max(max((min(r)+1 for r in requirement),default=0),escape,(len(needs)+len(ds)-1)//len(ds))
    if l>limit:return 999
    def ok(h):
        right={}
        opts=[[(i,t) for i,d in enumerate(a) for t in range(d,h)] for a in requirement]
        def rec(j,seen):
            for a in opts[j]:
                if a in seen:continue
                seen.add(a)
                if a not in right or rec(right[a],seen):
                    right[a]=j;return True
            return False
        return all(rec(j,set()) for j in sorted(range(len(opts)),key=lambda i:len(opts[i])))
    if not ok(limit):return 999
    rr=limit
    while l<rr:
        z=(l+rr)//2
        if ok(z):rr=z
        else:l=z+1
    return l

def examine(hist,meta):
    rot,shift,stop=map(int,meta.splitlines()[0].split())
    q=lambda i,j: i*30+j if not rot else j*30+i
    for t in (205,225):
        pp,hh,wall=hist[t]
        hs=list(map(idx,hh));ps=list(map(idx,pp))
        old=sum(B[idx(p)] for p in wall);opened=ALL^old
        human=sum({B[p] for p in hs});pet=sum({B[p] for p in ps});blocked=pet|wave(pet)
        reachable=flood(human,opened)
        ds=[dist(p,opened) for p in hs]
        for r in range(2,27,2):
            row=sum(B[q(r,c)] for c in range(30))
            missing=list(cells(row&opened))
            pbad=sum(bool(B[p]&blocked) for p in missing)
            hbad=sum(bool(B[p]&human) for p in missing)
            for left in (True,False):
                keep=range(r) if left else range(r+1,30)
                a=sum(B[q(j,shift)] for j in keep)
                passable=(opened&~row)&reachable
                seen=0
                while a&~seen:
                    seed=(a&~seen)&-(a&~seen)
                    comp=flood(seed,passable);seen|=comp|seed
                    if not comp:continue
                    pets=sum(bool(comp&B[p]) for p in ps)
                    area=comp.bit_count()
                    low=lower(missing,ds,comp) if area>=120 and pets<=1 else 999
                    yield [t,r,'before' if left else 'after',area,pets,len(missing),pbad,hbad,low,area/(2**pets),'+'.join(map(str,missing))]

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
    out.writerow(['case','turn','brace_row','side','area','reachable_pets','missing_contiguous_wall_jobs',
                  'currently_pet_blocked_jobs','currently_human_blocked_jobs','worker_time_lower_bound_if_qualified',
                  'peak_area_over_pet_penalty','missing_cells'])
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
            _,hist,_=observed(case,txt)
            out.writerows((f.name,*x) for x in examine(hist,meta.read_text()));sys.stdout.flush()
if __name__=='__main__':main()
