"""Derive actual endgame time from exact AHC008 first-separation records."""
import argparse,csv,gzip,json,statistics
from collections import Counter,defaultdict

def observation(r):
    total=int(r['animals'])
    observed={}
    for s in r['captures'].split(';'):
        if s:
            t,i,*_=map(int,s.split(':'))
            assert 0<t<=300 and 0<=i<total and i not in observed
            observed[i]=t
    events=Counter(observed.values())
    rem=total; timeline=[rem]
    for t in range(1,301):
        rem-=events[t]; timeline.append(rem)
    assert rem==int(r['final_reachable'])
    for t in (100,160,225,275):
        k=f'residual_turn{t}'
        if k in r: assert timeline[t]==int(r[k]),(r['case'],t)
    assert int(rem==0)==int(r['all_clear'])
    def first(check):return next((i for i,n in enumerate(timeline) if check(n)),-1)
    return dict(case=r['case'],model=r['model'],final=rem,
        le2=first(lambda n:n<=2),exact2=first(lambda n:n==2),
        le1=first(lambda n:n<=1),exact1=first(lambda n:n==1),
        clear=first(lambda n:n==0),
        actionable_turns_at_one=sum(n==1 for n in timeline[:300]),
        actionable_turns_at_two=sum(n==2 for n in timeline[:300]),
        jump_to_zero=int(rem==0 and all(n!=1 for n in timeline)))

def summarize(v):
    f=lambda vals:statistics.median(vals) if vals else None
    groups=defaultdict(list)
    for r in v:groups[r['final'] if r['final']<=2 else '3+'].append(r)
    q={}
    for k,s in groups.items():
        q[str(k)]={'cases':len(s),'median_first_le2':f([r['le2'] for r in s if r['le2']>=0]),
           'median_first_exact1':f([r['exact1'] for r in s if r['exact1']>=0]),
           'exact1_by250':sum(0<=r['exact1']<=250 for r in s),
           'exact1_after275':sum(r['exact1']>275 for r in s),
           'jump_to_zero':sum(r['jump_to_zero'] for r in s),
           'median_actionable_turns_at_one':f([r['actionable_turns_at_one'] for r in s])}
    return {
       'by_final_residual':q,
       'threshold_snapshots':{
          str(t):{k:[len(z),sum(r['final']==0 for r in z)]
             for k in ('le1','le2','exact1') for z in ([r for r in v if 0<=r[k]<=t],)}
          for t in (225,250,275)},
       'retrospective_solve_one_if_no_losses':sum(r['final']<=1 for r in v),
       'retrospective_solve_two_if_no_losses':sum(r['final']<=2 for r in v)
    }

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('ledger')
    p.add_argument('--cases',help='Write one milestone record per case and model')
    args=p.parse_args()
    op=gzip.open if args.ledger.endswith('.gz') else open
    with op(args.ledger,'rt') as f:
        rows=[observation(r) for r in csv.DictReader(f)]
    arms=sorted({r['model'] for r in rows})
    if args.cases:
        with open(args.cases,'w') as f:
            w=csv.DictWriter(f,list(rows[0]));w.writeheader();w.writerows(rows)
    print(json.dumps({a:summarize([r for r in rows if r['model']==a]) for a in arms},indent=2))
