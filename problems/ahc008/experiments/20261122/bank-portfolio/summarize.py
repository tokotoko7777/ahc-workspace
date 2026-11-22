import collections,csv,json,sys,statistics
d=collections.defaultdict(dict)
for r in csv.DictReader(open(sys.argv[1])):d[r['case']][r['model']]=r
rows=[x for x in d.values() if len(x)==2]
def report(z):
 q={'n':len(z),'byte_equal':sum(x['tactical']['actions_sha256']==x['portfolio']['actions_sha256'] for x in z)}
 outcomes=[int(x['portfolio']['score'])-int(x['tactical']['score']) for x in z]
 q['score_w_t_l']=[sum(x>0 for x in outcomes),sum(x==0 for x in outcomes),sum(x<0 for x in outcomes)]
 for model in ('tactical','portfolio'):
  rs=[x[model]for x in z]
  q[model]={k:sum(int(r[k]) for r in rs if r[k])for k in ('score','safe_humans','distinct_safe_area','all_clear','residual','final4','bank_capture4','bank_cow','bank_pig','bank_rabbit','bank_dog','bank_cat')}
  a=sorted(int(w)for r in rs for w in r.get('safe_person_areas','').split())
  q[model]['max_wall']=max((float(r['sec'])for r in rs),default=0.)
  q[model]['mean_safe_area_all_humans']=statistics.mean(a) if a else None
  q[model]['safe_area_quantiles']={str(q):a[(len(a)-1)*q//100] for q in (0,10,25,50,75,90,100)} if a else None
 q['cases']=[{'case':x['tactical']['case'],'n':int(x['tactical']['n']),'m':int(x['tactical']['m']),'score_delta':yy,
              'old_safe_people':int(x['tactical']['safe_humans']),'new_safe_people':int(x['portfolio']['safe_humans'])}
             for x,yy in zip(z,outcomes)if yy]
 return q
print(json.dumps({k:report(z) for k,z in [('overall',rows),('eligible',[x for x in rows if x['tactical']['use_bank']=='1']),('other',[x for x in rows if x['tactical']['use_bank']=='0'])]},indent=2))
