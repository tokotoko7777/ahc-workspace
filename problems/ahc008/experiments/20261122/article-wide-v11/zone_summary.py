import csv,collections,json,sys
data=list(csv.DictReader(open(sys.argv[1])))
checks={r['case']:r['actions_sha256'] for r in csv.DictReader(open(sys.argv[2])) if r['model']=='article'}
assert len(data)==2*len(checks) and all(checks[r['case']]==r['actions_sha256'] for r in data)
n=collections.Counter(); av=collections.defaultdict(collections.Counter)
for r in data:
 if r['model']!='article':continue
 for zone in json.loads(r['zones']).values():
  label='initial_passive_present' if any(zone['initial_ids'][:3]) else 'no_initial_passive_in_body'
  f=av[label];n[label]+=1
  for k,v in zone.items():
   if isinstance(v,int):f[k]+=v
   else:
    for i,z in enumerate(v,1):f[f'{k}_{i}']+=z
print(json.dumps({'zones':n,'stats':av},indent=2))
