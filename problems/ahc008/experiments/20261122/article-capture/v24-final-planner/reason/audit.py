"""Audits when the final one-or-two-pet planner finds or abandons a shape."""
import csv, hashlib, json, sys, tempfile
from pathlib import Path
W=Path.cwd();H=W/'ahc008_focus_solver/problems/ahc008'
sys.path.insert(0,str(H))
from benchmark_cases import run
T=W/'ahc008_focus_assets/tools/target/release/tester'
DIR=Path(__file__).resolve().parent
P=W/'ahc008_v24_five_retention'
r={q['case']:q for q in csv.DictReader((P/'dev30.csv').open()) if q['model']=='baseline'}
fields='turn left area old_begin retry adopted abandoned placed closed new_begin tier in_old on_old override'.split()
rows=[]
for f in sorted((P/'m5-dev30/in').glob('*.txt')):
 raw=f.read_bytes(); known=r[f.name]
 assert hashlib.sha256(raw).hexdigest()==known['input_sha256']
 with tempfile.TemporaryDirectory() as d:
  p=Path(d)/'reasons'
  a,err,rc,_=run([str(T),'env','AHC008_END_REASON='+str(p),str(DIR/'solve')],raw,4.5)
  assert rc==0 and err.decode().strip()==f"Score = {known['score']}"
  assert hashlib.sha256(a).hexdigest()==known['actions_sha256'],f.name
  decisions=[dict(zip(fields,map(int,z))) for z in csv.reader(p.open())]
 miss=int(known['final_reachable'])
 rows.append(dict(case=f.name,miss=miss,reachable_final_area=float(known['mean_human_floor']),decisions=decisions))
 if miss:
  print(f.name,miss,decisions[0]['turn'] if decisions else None,
        sum(z['old_begin']==-1 and z['turn']>=z['retry'] and not z['adopted'] for z in decisions),
        sum(z['adopted'] for z in decisions),sum(z['abandoned'] for z in decisions),
        sum(z['placed'] for z in decisions),sum(z['closed'] for z in decisions),flush=True)
(DIR/'result.json').write_text(json.dumps(rows,indent=2)+'\n')
