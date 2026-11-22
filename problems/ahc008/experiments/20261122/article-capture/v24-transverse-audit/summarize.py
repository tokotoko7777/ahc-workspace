import json,sys
from collections import Counter
cases=json.load(open(sys.argv[1]))
assert len(cases)==30
for shape in ('natural','old_plug','new_plug','destroys_live_rail'):
  n=Counter()
  ids={k:set() for k in ('visit','legal','otherwise_missed','idle_eta2','idle_now','prior_prep_followed_by_visit','prior_prep_followed_by_legal_eta2')}
  for c in cases:
    n['geometry']+=c['counts'].get(shape+'_geometry',0)
    for e in c['opportunities']:
      if e['shape']!=shape:continue
      n['prebuild_turns']+=bool(e['prebuild_now'])
      for what,ps in (
        ('visit',e['targets']),('legal',e['legal']),('otherwise_missed',e['unresolved']),
        ('idle_eta2',e['unresolved'] if e['eta']!=None and e['eta']<=2 else []),
        ('idle_now',e['unresolved'] if e['eta']==0 else []),
        ('prior_prep_followed_by_visit',e['targets'] if e['earliest_prior_prebuild']!=None else []),
        ('prior_prep_followed_by_legal_eta2',e['unresolved'] if e['earliest_prior_prebuild']!=None and e['eta']!=None and e['eta']<=2 else []),
      ):ids[what].update((c['case'],i) for i in ps)
  print(shape,n, {k:[len(v),len({c for c,i in v})] for k,v in ids.items()})
