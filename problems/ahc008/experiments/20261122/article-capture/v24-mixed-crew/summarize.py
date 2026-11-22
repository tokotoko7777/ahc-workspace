"""Compare two official/replay benchmark arms with strict within-case inputs."""
import csv,json,sys
from collections import Counter
from pathlib import Path
def flag(x):
 return bool(int(x['all_clear'])) and float(x['mean_human_floor'])>=300
def stats(rows):
 return dict(n=len(rows),score=sum(int(x['score']) for x in rows),
             full=sum(int(x['all_clear']) for x in rows),useful=sum(flag(x) for x in rows),
             residual=sum(int(x['final_reachable']) for x in rows),
             max_sec=max((float(x['wall_sec']) for x in rows),default=0))
def report(file):
 z=list(csv.DictReader(Path(file).open()))
 a={r['case']:r for r in z if r['model']=='baseline'}
 b={r['case']:r for r in z if r['model']=='article'}
 assert a.keys()==b.keys()
 for k in a: assert a[k]['input_sha256']==b[k]['input_sha256'] and a[k]['humans']==b[k]['humans']
 out={}
 for name,filter_fn in [('all',lambda x:True),('M5',lambda x:x['humans']=='5'),('nonM5',lambda x:x['humans']!='5')]:
  keys=[k for k,r in a.items() if filter_fn(r)]
  er=[k for k in keys if a[k]['actions_sha256']!=b[k]['actions_sha256']]
  out[name]=dict(baseline=stats([a[k] for k in keys]),candidate=stats([b[k] for k in keys]),changed=er)
  if name=='nonM5':
   out[name]['changed_detail']=[[k,a[k]['score'],b[k]['score'],a[k]['terry'],b[k]['terry']] for k in er]
 return out
for f in sys.argv[1:]: print(json.dumps({f:report(f)},indent=2))
