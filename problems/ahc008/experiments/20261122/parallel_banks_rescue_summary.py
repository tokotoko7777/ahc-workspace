"""Paired rescue benchmarks, including the exact non-trigger cohort."""
import argparse,csv,json,statistics
from collections import defaultdict
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('csv',type=Path)
a=p.parse_args()
data=defaultdict(dict)
for row in csv.DictReader(a.csv.open()):
    data[row['model']][row['case']]=row
trig={case for case,row in data['rescue160'].items() if int(row['cutoff'])==160}
def n(row,key):return int(row.get(key) or 0)
def final_four(row):return sum(n(row,f'final4_{i}') for i in range(1,6))
def summary(rs):
    r=dict(cases=len(rs),score=sum(n(x,'score') for x in rs),max_wall=max(float(x['time']) for x in rs),
           final4=sum(map(final_four,rs)),direct_bank=sum(n(x,'bank_capture4') for x in rs),
           final4_species=[sum(n(x,f'final4_{k}') for x in rs) for k in range(1,6)],
           direct_species=[sum(n(x,f'bank_species_{k}') for x in rs) for k in range(1,6)],
           total_residual_at_own_cutoff=sum(n(x,'residual_at_release') for x in rs),
           final_residual=sum(n(x,'final_residual_pets') for x in rs),
           final_safe_union=sum(n(x,'final_safe_union') for x in rs),
           cleared_humans=sum(n(x,'cleared_humans') for x in rs),
           humans=sum(n(x,'n_humans') for x in rs),
           all_clear=sum(n(x,'final_residual_pets')==0 for x in rs),
           mean_safe_area_per_human=statistics.mean(float(x['mean_clean_area_per_human']) for x in rs))
    return r
m={}
for model,x in data.items():
    m[model]={key:summary(rows) for key,rows in (
        ('all',list(x.values())),('triggered',[r for c,r in x.items() if c in trig]),
        ('omitted',[r for c,r in x.items() if c not in trig])) if rows}
def paired(a,b,key):
    x,y=data[a],data[b];v=[key(y[k])-key(x[k]) for k in x]
    return dict(wins=sum(z>0 for z in v),ties=sum(z==0 for z in v),losses=sum(z<0 for z in v),delta=sum(v))
for a,b in [('stay225','rescue160'),('tactical','rescue160')]:
    if a not in data:continue
    m[a+'→'+b]=dict(score=paired(a,b,lambda r:n(r,'score')),final4=paired(a,b,final_four),
                   fewer_residual=paired(a,b,lambda r:-n(r,'final_residual_pets')),
                   final_safe_union=paired(a,b,lambda r:n(r,'final_safe_union')))
print(json.dumps(m,indent=2,ensure_ascii=False))
