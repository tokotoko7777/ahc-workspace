import csv,collections,statistics,sys,json,random
a=list(csv.DictReader(open(sys.argv[1])))
keys=('score','bank_capture4','final_residual_pets','residual160','safe_union','cleared_humans','n_tubes')
def measure(q):
 out={c:sum(int(x[c]) for x in q) for c in keys if c in q[0]}
 out.update(trigger=sum(x['release']=='160' for x in q), allclear=sum(x['final_residual_pets']=='0' for x in q), time_median=statistics.median(float(x['time']) for x in q), time_max=max(float(x['time']) for x in q))
 out['bank_species']=[sum(int(x[f'bank_species_{i}']) for x in q) for i in range(1,6)]
 out['final4_species']=[sum(int(x[f'final4_{i}']) for x in q) for i in range(1,6)]
 return out
bases=list(zip(a[::2],a[1::2]))
summary={}
for group, predicate in (('all',lambda m:True), ('eligible',lambda m:m>=7), ('inactive',lambda m:m<7)):
 pair=[(x,y) for x,y in bases if predicate(int(x['workers']))]
 if not pair: continue
 base,changed=zip(*pair)
 d=[int(y['score'])-int(x['score']) for x,y in pair]
 n=len(pair);rng=random.Random(0x008)
 ds=sorted(sum(rng.choices(d,k=n)) for _ in range(20000))
 summary[group]=dict(cases=n, rescue3=measure(base),wide_rescue=measure(changed),win_tie_loss=[sum(x>0 for x in d),sum(x==0 for x in d),sum(x<0 for x in d)],resampled_95_sum=[ds[500],ds[19499]],bytes_same=sum(x['actions_sha256']==y['actions_sha256'] for x,y in pair), clearance_wins_losses=[sum(x['final_residual_pets']!='0' and y['final_residual_pets']=='0' for x,y in pair),sum(x['final_residual_pets']=='0' and y['final_residual_pets']!='0' for x,y in pair)])
print(json.dumps(summary,indent=2))
