import csv,collections,sys,json,statistics
species=['cow','pig','rabbit','dog','cat']
S=['original','edge']
d=collections.defaultdict(dict)
for row in csv.DictReader(open(sys.argv[1])):d[row['case']][row['solver']]=row
pairs=[x for x in d.values()if len(x)==2]
def summary(pairs):
 out={'n':len(pairs),'score_w_t_l':[0]*3,'byte_identical':0,'models':{}}
 for p in pairs:
  a,b=[p[x] for x in S]
  n0,n1=int(a['score']),int(b['score']);out['score_w_t_l'][0 if n1>n0 else 1 if n1==n0 else 2]+=1
  out['byte_identical']+=a['action_sha256']==b['action_sha256']
 for arm in S:
  rs=[x[arm]for x in pairs]
  get=lambda k:sum(int(r[k])for r in rs if r.get(k))
  humans=sum(sum(int(r[k])for k in ('wait','walk','build'))//300 for r in rs)
  out['models'][arm]={
   'score':get('score'),'all_clear':sum(all(int(x['residual_'+k])==0 for k in species)for x in rs),
   'clear_humans':get('clear_humans'),'humans':humans,
   'area_all_human_mean':get('human_safe_area_sum')/max(humans,1),
   'distinct_safe_area':get('distinct_safe_area'),
   'residual_species':[get('residual_'+k)for k in species],'exact4_species':[get('exact4_'+k)for k in species],
   'wait_walk_build':[get(k)for k in ('wait','walk','build')],
   'prefix':{k:get('prefix'+('_'+k if k else ''))for k in ('','wall','guard','build','walk','wait','exact_dog','exact_other')},
   'max_wall':max((float(x['direct_wall_sec'])for x in rs if x.get('direct_wall_sec')),default=None),
  }
 return out
print(json.dumps({key:summary(rows) for key,rows in (
 ('overall',pairs),('one',[x for x in pairs if int(x['original']['dogs'])==1]),('other',[x for x in pairs if int(x['original']['dogs'])!=1])
)},indent=2))
