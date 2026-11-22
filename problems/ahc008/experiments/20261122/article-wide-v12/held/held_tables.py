"""Like-for-like held comparison by predeclared visible worker counts."""
import csv,statistics,sys
for p in sys.argv[1:]:
 rows=list(csv.DictReader(open(p)))
 assert len(rows)==200
 print(p)
 for name,f in [('all',lambda m:True),('M5-6',lambda m:m<=6),('M7-10',lambda m:m>=7)]:
  pairs=[(a,b) for a,b in zip(rows[::2],rows[1::2]) if f(int(a['humans']))]
  print(name,len(pairs))
  for j in [0,1]:
   r=[q[j] for q in pairs]
   print(j, [sum(int(z[k]) for z in r) for k in ['score','all_clear','final_reachable','exact4_first','residual_turn100','residual_turn160','residual_turn225','residual_turn275']], 
    [sum(z['all_clear']=='1' and int(z['clean_union'])>=lim for z in r) for lim in (100,300,450)],
    statistics.mean(float(z['mean_human_floor']) for z in r), max(float(z['wall_sec']) for z in r))
  print([(b['case'], b['clean_union']) for a,b in pairs if a['all_clear']!=b['all_clear'] and b['all_clear']=='1'])
