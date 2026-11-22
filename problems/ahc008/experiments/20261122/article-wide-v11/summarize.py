"""Paired realized broad coverage, with separate worker-count regimes."""
from collections import Counter
import csv,json,statistics,sys
r=list(csv.DictReader(open(sys.argv[1])))
p=list(zip(r[::2],r[1::2]))
assert all(a['case']==b['case'] and a['input_sha256']==b['input_sha256'] and a['model']=='baseline' and b['model']=='article' for a,b in p)
out={}
stats=lambda v:[min(v),statistics.median(v),max(v)] if v else None
for group,idx in [('all',lambda n:True),('M5-6',lambda n:n<=6),('M7-10',lambda n:n>=7)]:
    q=[(a,b) for a,b in p if idx(int(a['humans']))]
    if not q:continue
    ans={}
    out[group]={'cases':len(q),'score_WTL':[sum((int(b['score'])>int(a['score']))-(int(b['score'])<int(a['score']))==s for a,b in q) for s in (1,0,-1)],
       'clear_gained':[b['case'] for a,b in q if int(b['all_clear'])>int(a['all_clear'])],
       'clear_lost':[a['case'] for a,b in q if int(b['all_clear'])<int(a['all_clear'])], 'arms':ans}
    for j,k in enumerate(('baseline','article')):
        rows=[s[j] for s in q]; stages=[json.loads(x['stages']) for x in rows]
        ev=[list(map(int,e.split(':'))) for x in rows for e in x['captures'].split(';') if e]
        exact=[x for x in ev if x[3]==4]
        ans[k]=dict({m:sum(int(a[m]) for a in rows) for m in ['score','all_clear','final_reachable','exact4_first','final_exact4','residual_turn100','residual_turn160','residual_turn225','residual_turn275','clean_union']},
            exact4_species=dict(Counter(knd for t,i,knd,ar,s in exact)),
            exact4_at100=sum(t<=100 for t,*_ in exact), exact4_at160=sum(t<=160 for t,*_ in exact),
            floor=[statistics.mean(float(x['mean_human_floor']) for x in rows),stats([float(x['mean_human_floor']) for x in rows if x['all_clear']=='1'])],
            wall=stats([float(x['wall_sec']) for x in rows]),
            clear_area_100=sum(int(x['clean_union'])>=100 and x['all_clear']=='1' for x in rows),
            clear_area_300=sum(int(x['clean_union'])>=300 and x['all_clear']=='1' for x in rows),
            first5=stats([min(v) for s in stages if (v:=[i for i in s['ready'] if i>=0])]),
            first_staffed5=stats([min(v) for s in stages if (v:=[i for i in s['staff'] if i>=0])]),
            first_lock=stats([min(v) for s in stages if (v:=[i for i in s['first_lock'] if i>=0])]),
            milestones={t:{m:stats([s['milestones'][str(t)][m] for s in stages]) for m in ('shells','open_tubes','staffed_tubes','ready_banks')} for t in (80,100,150)},
            t150_total_shells_by_bank=[sum(s['milestones']['150']['bank_shells'][i] for s in stages if len(s['milestones']['150']['bank_shells'])>i) for i in range(4)])
print(json.dumps(out,indent=2))
