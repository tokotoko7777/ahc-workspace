"""Summarize official verified quad benchmarks and paired count/score deltas."""
import argparse,csv,json,statistics
from collections import defaultdict
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('csv',type=Path)
a=p.parse_args()
groups=defaultdict(dict)
for x in csv.DictReader(a.csv.open()):
    groups[x['model']][x['case']]=x
def intval(row,key):
    return int(row.get(key) or 0)
def sumkey(rows,key):
    return sum(intval(r,key) for r in rows)
def exact(row):
    return sum(intval(row,f'final4_{i}') for i in range(1,6))
result={}
for label,bycase in groups.items():
    rows=list(bycase.values())
    a=dict(cases=len(rows),score_sum=sumkey(rows,'score'),
           max_wall_s=max(float(x['wall_s']) for x in rows),
           median_wall_s=statistics.median(float(x['wall_s']) for x in rows),
           final_four=sum(map(exact,rows)),
           final_four_species=[sumkey(rows,f'final4_{i}') for i in range(1,6)],
           residual_pets=sumkey(rows,'final_reachable'),
           all_clear=sum(intval(r,'final_reachable')==0 for r in rows))
    if label!='control':
        a.update({k:sumkey(rows,k) for k in ('capture4','full_preparation_zero','full_legal_ready',
                  'full_legal_two_or_more_gates','shell_built','active_crew_moves','early_crew_waits',
                  'diverted_commands','diverted_original_walls')})
        a['seals']=sum(intval(r,'seal')>=0 for r in rows)
        a['multiwall_seals']=sum(intval(r,'final_jump')>=2 for r in rows)
        a['capture_species']=[sumkey(rows,f'capture_{i}') for i in range(1,6)]
        a['median_ready']=statistics.median(intval(r,'ready_turn') for r in rows if intval(r,'ready_turn')>=0)
    result[label]=a
pair={}
for a,b in [('fixed-square','deferred-square'),('control','deferred-square')]:
    if a not in groups or b not in groups:continue
    assert set(groups[a])==set(groups[b])
    da,db=groups[a],groups[b]
    assert all(da[c]['input_sha256']==db[c]['input_sha256'] for c in da)
    x={}
    for label,measure in [('score',lambda r:intval(r,'score')),
                          ('final_four',exact),('fewer_residual',lambda r:-intval(r,'final_reachable'))]:
        delta=[measure(db[k])-measure(da[k]) for k in da]
        x[label]=dict(net=sum(delta),win=sum(v>0 for v in delta),tie=sum(v==0 for v in delta),loss=sum(v<0 for v in delta))
    pair[a+'→'+b]=x
print(json.dumps(dict(models=result,paired=pair),indent=2))
