"""Exhaustive actual staffed one/two wall full-clear now on v7 misses."""
import argparse,csv,hashlib,json,sys,tempfile,collections,itertools
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,flood,wave
from parallel_banks_anchored_cut_audit import buf,bits,rect

def hunt(turn,pp,hh,wall):
    root=flood(B[hh[0]],ALL^wall)
    crew=buf(hh);actual=[i for i,p in enumerate(pp) if B[p]&root]
    if crew&root!=crew:return None,'crew_already_split'
    if not 1<=len(actual)<=2:return None,'not_1_2'
    if root.bit_count()<300:return None,'small_current'
    pet=buf(pp);b=crew|pet|wave(pet)
    ass={}
    for i,h in enumerate(hh):
        for q in bits(wave(B[h])&root&~b):
            ass.setdefault(q,[]).append(i)
    candidates=list(sorted(ass))
    opt=[];jobs=0
    for k in (1,2):
        for walls in itertools.combinations(candidates,k):
            if k==2:
                a,c=walls
                match=next(((i,j) for i in ass[a] for j in ass[c] if i!=j),None)
                if match is None:continue
            else:
                match=(ass[walls[0]][0],)
            jobs+=1
            m=buf(walls)
            keep=flood(B[hh[0]],root&~m)
            if keep&crew!=crew or keep&pet:continue
            remove=root.bit_count()-keep.bit_count()
            if keep.bit_count()<300 or keep.bit_count()*100<65*root.bit_count():continue
            comp={}
            opened=ALL^(wall|m)
            for i in actual:
                pen=flood(B[pp[i]],opened)
                c=pen&-pen
                data=comp.setdefault(c,dict(area=pen.bit_count(),pet_ids=[],nonrect=not rect(pen)))
                assert not pen&crew
                data['pet_ids'].append(i)
            opt.append(dict(wall=list(walls),people=match,kept=keep.bit_count(),loss=remove,pens=list(comp.values())))
    return dict(turn=turn,current_root=root.bit_count(),reachable=actual,legal_cells=len(ass),
                legal_sets=jobs,positive=len(opt),
                best=min(opt,key=lambda x:(x['loss'],len(x['wall']),x['wall'])) if opt else None),'tested'

def selfcheck():
    # True four-cell pen at top-left requires one currently legal mouth.
    p={0,1,30,31}
    door=2
    wall=buf(q for c in p for q in bits(wave(B[c])) if q not in p and q!=door)
    x,k=hunt(275,[30],[3,33,90,99,180],wall)
    assert k=='tested' and x['best']['wall']==[2] and x['best']['pens'][0]['area']==4
    x,_=hunt(275,[1],[3,33,90,99,180],wall)
    assert x['best'] is None
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','tester','solver','cases','reference','events','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    selfcheck()
    with open(a.reference) as f:refs=[r for r in csv.DictReader(f) if r['model']=='article' and int(r['final_reachable'])>0]
    rows=[];why=collections.Counter()
    for r in refs:
        n=r['case'];data=(a.cases/n).read_bytes();tool=a.tester.resolve()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);act=d/'actions';tr=d/'trace'
            o,e,rc,_=run([tool,a.solver.resolve()],data,3.)
            assert not rc and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([tool,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tr],data,10.)
            assert (po,pe,pr)==(o,e,0)
            tape=tr.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(data.decode(),tape,cmds,detailed=True)[0]==int(r['score'])
            _,hist,_=observed(data,tape)
            for t in range(260,300):
                pp,hh,w=hist[t]
                x,k=hunt(t,list(map(idx,pp)),list(map(idx,hh)),buf(map(idx,w)))
                why[k]+=1
                if x:rows.append(dict(x,case=n,final_baseline=int(r['final_reachable'])))
        print(n,flush=True)
    good=[r for r in rows if r['best']]
    z=dict(verified_final_miss_cases=len(refs),state_reasons=why,actual_tested_cases=len({r['case'] for r in rows}),
           legal_sets=sum(r['legal_sets'] for r in rows),positive_sets=sum(r['positive'] for r in good),
           positive_turns=len(good),positive_cases=len({r['case'] for r in good}),
           first_before299_cases=len({r['case'] for r in good if r['turn']<299}),first=next(iter(good),None))
    a.events.write_text(''.join(json.dumps(x,separators=(',',':'))+'\n' for x in rows))
    a.summary.write_text(json.dumps(z,indent=2)+'\n');print(json.dumps(z,indent=2))
if __name__=='__main__':main()
