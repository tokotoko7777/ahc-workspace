import json,sys
from collections import Counter
rows=json.load(open(sys.argv[1]))
assert len(rows)==30
print('scores',sum(r['score'] for r in rows),'exact',sum(r['exact'] for r in rows))
for eta in [0,1]:
 cutcases=set();visitor=set();legal=set();near=set();ready=set();times=Counter();total=0
 for r in rows:
  for x in r['cuts']:
   if x['prepare_eta']!=eta:continue
   total+=1;times[x['turn']]+=1;cutcases.add(r['case'])
   visitor.update((r['case'],i) for i in x['visitors'])
   for t,ids,(d,w) in x['windows']:
    legal.update((r['case'],i) for i in ids)
    if d<=2:near.update((r['case'],i) for i in ids)
    if d==0:ready.update((r['case'],i) for i in ids)
 fmt=lambda v:(len(v),len({c for c,i in v}))
 print(eta,'topology',total,len(cutcases),dict(times),'visitor',fmt(visitor),'legal',fmt(legal),
    'near idle ≤2',fmt(near),'adjacent now',fmt(ready))
