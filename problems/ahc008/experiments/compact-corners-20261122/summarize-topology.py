import sys,csv,json,collections,statistics
for path in sys.argv[1:]:
 print(path)
 rows=collections.defaultdict(list)
 for r in csv.DictReader(open(path)):rows[r['model']].append(r)
 bykey=collections.defaultdict(dict)
 for label, rr in rows.items():
  for r in rr: bykey[r['case']][label]=r
 np=int
 for label, zz in rows.items():
  clear=[z for z in zz if np(z['distinct_residual_pets'])==0]
  pens=[(z['case'],area,pc) for z in zz for h,area,pc in json.loads(z['groups_humans_area_pets']) if h==0 and pc and area<=32]
  print(label,len(zz),'clear',len(clear),'remaining',sum(np(z['distinct_residual_pets']) for z in zz),'small pets',sum(p[2] for p in pens),
    'pens',len(pens),'pen mean',statistics.mean(a for _,a,_ in pens) if pens else None,
    'clear animal area',statistics.mean(np(z['distinct_pet_only_area']) for z in clear) if clear else None)
  print(pens)
 cols=list(rows)
 if len(cols)>=2:
  for label in cols[1:]:
   a=[(v[cols[0]],v[label]) for v in bykey.values() if cols[0] in v and label in v and np(v[cols[0]]['distinct_residual_pets'])+np(v[label]['distinct_residual_pets'])==0]
   print(label,'paired all clear',len(a),'animal-area reduction',statistics.mean(np(x['distinct_pet_only_area'])-np(y['distinct_pet_only_area']) for x,y in a) if a else None)
