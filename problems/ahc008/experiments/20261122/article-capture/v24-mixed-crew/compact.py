import csv,gzip,sys
from io import TextIOWrapper
cols='case,model,input_sha256,actions_sha256,score,wall_sec,animals,humans,first_all_clear,mean_human_floor,all_clear,final_reachable,final_exact4,residual_turn100,residual_turn160,residual_turn225,residual_turn275,captures,terry,intervention_turns'.split(',')
with gzip.GzipFile(filename=sys.argv[2],mode='wb',mtime=0) as b:
 with TextIOWrapper(b,encoding='utf-8',newline='') as w:
  f=csv.DictWriter(w,cols,extrasaction='ignore',lineterminator='\n');f.writeheader()
  for row in csv.DictReader(open(sys.argv[1])):
   row['intervention_turns']=';'.join(z.split()[0] for z in row['telemetry'].split(';') if z and z.split()[1]=='10')
   f.writerow(row)
