"""Compare article-policy replay ledgers on the same exact case inputs."""
from collections import Counter
import csv,gzip,json,statistics,sys

def read(path):
    fn=gzip.open if path.endswith('.gz') else open
    with fn(path,'rt') as f:rows=list(csv.DictReader(f))
    d={}
    for r in rows:
        d.setdefault((r['case'],r['input_sha256']),{})[r.pop('model')]=r
    assert all(set(x)=={'baseline','article'} for x in d.values())
    return d

def headline(a):
    intfields=('score','all_clear','final_reachable','final_exact4','exact_nonrect4to48_first',
     'residual_turn100','residual_turn160','residual_turn225','residual_turn275',
     'area_capacity_turn160','area_capacity_turn225','area_capacity_turn300','area_capacity_release')
    head={k:sum(int(r[k]) for r in a) for k in intfields if k in a[0]}
    approximate=sum(float(r['mean_human_floor']) for r in a)*1e8/900
    head['final_area_capacity_rounded_mean_floor_approx']=round(approximate)
    head['median_wall']=statistics.median(float(r['wall_sec']) for r in a)
    head['max_wall']=max(float(r['wall_sec']) for r in a)
    modes=Counter();totals=Counter();clears=Counter();ownership=Counter()
    for r in a:
        by={int(z.split()[0]):int(z.split()[1]) for z in r['telemetry'].split(';') if z}
        t0=min((t for t,m in by.items() if m in (3,5)),default=-1)
        if t0>=0:
            events=tuple(int(z.split(':')[0]) for z in r['captures'].split(';') if z)
            ownership[int(r['animals'])-sum(t<=t0 for t in events)]+=1
        assert set(by.values()) <= set(range(6))
        totals.update(by.values())
        modes.update(by.get(int(z.split(':')[0])-1,-1) for z in r['captures'].split(';') if z)
        if r['all_clear']=='1':
            clears[by.get(int(r['first_all_clear'])-1,-1)]+=1
    head.update(captures_by_mode=dict(modes),all_clear_by_mode=dict(clears),
              turns_by_mode=dict(totals),first_last_planner_ownership_by_uncaught=dict(ownership))
    return head

def compare(a,b):
    di=[int(y['score'])-int(x['score']) for x,y in zip(a,b)]
    return {
        'score_wtl': [sum(x>0 for x in di),sum(x==0 for x in di),sum(x<0 for x in di)],
        'delta_sum':sum(di),'clear_transitions':dict(Counter(x['all_clear']+y['all_clear'] for x,y in zip(a,b))),
        'same_output':sum(x['actions_sha256']==y['actions_sha256'] for x,y in zip(a,b))}

if __name__=='__main__':
    compare_against=read(sys.argv[1])
    base=[v['baseline'] for v in compare_against.values()]
    prev=None
    print(json.dumps({'baseline':headline(base)},ensure_ascii=False))
    for p in sys.argv[1:]:
        run=read(p)
        assert set(run)==set(compare_against),(p,'case stream mismatch')
        assert all(run[k]['baseline']['actions_sha256']==v['baseline']['actions_sha256']
                  for k,v in compare_against.items()),(p,'baseline action mismatch')
        arm=[run[k]['article'] for k in compare_against]
        print(json.dumps({p:headline(arm),'versus baseline':compare(base,arm),
                          'versus previous ledger':compare(prev,arm) if prev else None},ensure_ascii=False))
        prev=arm
