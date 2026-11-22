"""Exact C/P/R survival under fixed observed two-job wall schedules."""
import argparse,csv,hashlib,json,sys,tempfile
from collections import defaultdict
from pathlib import Path
import numpy as np
from guarded_entry_trace import observed
from stochastic_lock_audit import ADJ,idx

V=900

class Motion:
    def __init__(self,wall):
        src=[];dst=[];weight=[]
        for p,nbrs in enumerate(ADJ):
            if p in wall:continue
            valid=[q for q in nbrs if q not in wall]
            if not valid:valid=[p]
            for q in valid:src.append(p);dst.append(q);weight.append(1/len(valid))
        self.src,self.dst,self.weight=map(np.asarray,(src,dst,weight))

    def step(self,d):
        return np.bincount(self.dst,weights=d[self.src]*self.weight,minlength=V)

def geometry(row,initial,species):
    p0,_,w0=initial
    pp=list(map(idx,p0))
    board=set(map(idx,w0))
    cut=row['walls'];target=row['pet'];finish=row['time']-1
    stages=defaultdict(set)
    for t,j,w,st in row['plan']:stages[t].add(cut[j])
    assert len(stages[finish]) and set.union(*stages.values())==set(cut)
    old=Motion(board);motion=old;passive=[j for j,k in enumerate(species) if k<=3]
    assert target in passive
    true={j:np.zeros(V,dtype=float) for j in passive}
    for j,d in true.items():d[pp[j]]=1.
    untouched=true[target].copy()
    scheduled_alive=[]
    for t in range(finish+1):
        if t in stages:
            wall=stages[t]
            forbidden={j for q in wall for j in [q,*ADJ[q]]}
            for d in true.values():d[list(forbidden)]=0.
            board.update(wall);motion=Motion(board)
            scheduled_alive.append([t,float(np.prod([d.sum() for d in true.values()]))])
        if t==finish:break
        for j,d in true.items():
            for step in range(species[j]):d=motion.step(d)
            true[j]=d
        for step in range(species[target]):untouched=old.step(untouched)
    pen=row['cells']
    p_target=float(true[target][pen].sum())
    p_other=float(np.prod([d.sum() for j,d in true.items() if j!=target]))
    p_untouched=float(untouched[pen].sum())
    assert 0<=p_target<=1+1e-12 and 0<=p_other<=1+1e-12 and abs(untouched.sum()-1)<1e-12
    return dict(case=row['case'],turn=row['turn'],pet=target,type=species[target],walls=cut,
           closure_wait=finish,pen_area=len(pen),target_insulation_and_own_legality=p_target,
           all_other_passive_legal=p_other,fixed_model_upper_probability=p_target*p_other,
           untouched_target_inside=p_untouched,survival_at_scheduled_stage=scheduled_alive)

def fixtures():
    # Hallway 0..3 plus two side cells 30,31; prospective gate is 1.
    # In one substep the cow at 0 reaches 1 or 30 and both are disallowed at
    # a simultaneous two-wall placement at 1,31. Test exact elimination.
    open_={0,1,2,3,30,31};m=Motion(set(range(V))-open_)
    vec=np.zeros(V);vec[0]=1
    step=m.step(vec)
    assert abs(step[1]-.5)<1.e-12 and abs(step[30]-.5)<1.e-12 and step.sum()==1.
    walls={1,31}
    for q in walls:
        step[[q,*ADJ[q]]]=0
    assert step.sum()==0.
    isolated=Motion(set(range(V))-{42})
    vec=np.zeros(V);vec[42]=1
    assert np.array_equal(isolated.step(vec),vec)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','tester','solver','cases','reference','geometries','out','summary'):p.add_argument(k,type=Path)
    a=p.parse_args();sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    fixtures()
    with open(a.reference) as f: ref={r['case']:r for r in csv.DictReader(f) if r['model']=='article'}
    with open(a.geometries) as f: ledger=[r for r in map(json.loads,f)
       if r['status']=='geometry' and r['within'] and len(r['walls'])>r['crew']]
    assert len(ledger)==14 and len({r['case'] for r in ledger})==9
    by=defaultdict(list)
    for r in ledger:by[r['case']].append(r)
    out=[]
    for name,rows in sorted(by.items()):
        c=(a.cases/name).read_bytes();ref0=ref[name];tester=a.tester.resolve()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);act=d/'actions';tape=d/'tape'
            o,e,rc,_=run([tester,a.solver.resolve()],c,3.)
            assert not rc and hashlib.sha256(o).hexdigest()==ref0['actions_sha256']
            assert e.strip()==('Score = '+ref0['score']).encode()
            act.write_bytes(o)
            po,pe,pr,_=run([tester,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tape],c,10.)
            assert (po,pe,pr)==(o,e,0)
            lines=tape.read_text().splitlines()
            assert replay(c.decode(),lines,o.decode().splitlines(),detailed=True)[0]==int(ref0['score'])
            species,hist,_=observed(c,lines)
            # Only current visible state drives probability calculation.
            for r in rows:out.append(geometry(r,hist[r['turn']],species))
        print(name,flush=True)
    best={name:max(r['fixed_model_upper_probability'] for r in out if r['case']==name) for name in by}
    result=dict(verified_cases=len(by),schedules=len(out),per_case_best=best,
           best_sum=sum(best.values()),max_probability=max(best.values()),
           improves_over_untouched=sum(r['target_insulation_and_own_legality']>r['untouched_target_inside'] for r in out),
           target_only_mean=np.mean([r['target_insulation_and_own_legality'] for r in out]),
           untouched_mean=np.mean([r['untouched_target_inside'] for r in out]))
    a.out.write_text(''.join(json.dumps(r,separators=(',',':'))+'\n' for r in out))
    a.summary.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
if __name__=='__main__':main()
