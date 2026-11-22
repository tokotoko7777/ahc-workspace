import csv,hashlib,json,sys,tempfile
from pathlib import Path
from replay_audit import inspect, W, H
from benchmark_cases import run
p=Path(__file__).resolve().parent
T=W/'ahc008_focus_assets/tools/target/release/tester'
x=W/'ahc008_v24_standby_early_seam'
ref={q['case']:q for q in csv.DictReader((x/'dev30.csv').open()) if q['model']=='article'}
out=[]; mismatch=[]
cases=sorted((W/'ahc008_v24_five_retention/m5-dev30/in').glob('*.txt'))
assert len(cases)==len(ref)==30
for f in cases:
 raw=f.read_bytes(); q=ref[f.name]
 assert q['input_sha256']==hashlib.sha256(raw).hexdigest()
 with tempfile.TemporaryDirectory() as d:
  d=Path(d);a=d/'cmd'; r=d/'replies'; l=d/'audit'
  data,e,rc,_=run([str(T),'env','AHC008_EARLY_SEAM_AUDIT='+str(l),str(p/'solve')],raw,4.5)
  assert rc==0
  a.write_bytes(data)
  b,er,rc,_=run([str(T),sys.executable,H/'benchmark_cases.py','--playback',a,r],raw,12.)
  assert (b,er,rc)==(data,e,0)
  score=inspect(raw,data.decode().splitlines(),r.read_text().splitlines(),l.read_text().splitlines())
  same=hashlib.sha256(data).hexdigest()==q['actions_sha256']
  if not same: mismatch.append(f.name)
  if same: assert score['score']==int(q['score'])
  assert e.decode().strip()==f"Score = {score['score']}"
  out.append(dict(case=f.name,exact=same,**score))
  print(f.name,same,len(score['early_episodes']),flush=True)
(p/'dev30.json').write_text(json.dumps(out,indent=2)+'\n')
print('different',mismatch)
