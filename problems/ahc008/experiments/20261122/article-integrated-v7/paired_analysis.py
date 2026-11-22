"""Actual paired all-clear, retained area and residuals on immutable v4/v7."""
import csv,gzip,hashlib,json,statistics,collections,sys
def read(p):
    fn=gzip.open if p.endswith('.gz') else open
    with fn(p,'rt') as f:return list(csv.DictReader(f))
new,old=map(read,sys.argv[1:3])
ref={r['case']:r for r in old if r['model']=='article'}
rows=collections.defaultdict(dict)
for r in new:rows[r['case']][r['model']]=r
assert len(rows)==len(ref)
c=collections.Counter();ts=[];bags=[collections.Counter(),collections.Counter()];gains=[];loss=[]
for name,pair in rows.items():
    a,b=pair['baseline'],pair['article']
    h=ref[name]
    assert a['input_sha256']==h['input_sha256'] and a['actions_sha256']==h['actions_sha256']
    assert a['prefix100']==b['prefix100']
    e=[list(map(int,z.split())) for z in b['bridge'].split(';') if z]
    count=collections.Counter(z[1] for z in e);c.update(count)
    c['activated_cases']+=bool(count[0])
    if not count[0]:
        assert a['actions_sha256']==b['actions_sha256']
        c['inactive_exact']+=1
    ac=a['all_clear']+b['all_clear'];c['clear_'+ac]+=1
    for k,r in enumerate((a,b)):bags[k][int(r['final_reachable'])]+=1
    if ac in ('01','10'):
        t=[]
        for s in b['captures'].split(';'):
            if s:t.append(list(map(int,s.split(':'))))
        q=dict(case=name,clear=ac,old_residual=int(a['final_reachable']),new_residual=int(b['final_reachable']),
            old_human_floor=float(a['mean_human_floor']),new_human_floor=float(b['mean_human_floor']),
            old_clean_floor=float(a['mean_clean_floor']),new_clean_floor=float(b['mean_clean_floor']),
            new_exact4=int(b['final_exact4']),new_total_caught=int(b['ever_caught']),
            max_first_isolation_pen=max((x[3] for x in t),default=0),
            new_first_isolation_areas=[x[3] for x in t],
            first_clear=int(b['first_all_clear']))
        (gains if ac=='01' else loss).append(q)
print(json.dumps(dict(verified=len(rows),counts=c,baseline_residual=bags[0],variant_residual=bags[1],
                     gained=gains,lost=loss),indent=2))
