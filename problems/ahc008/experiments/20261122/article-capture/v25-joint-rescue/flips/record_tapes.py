"""Record official tapes for the immutable selected held cases."""
import csv, hashlib,subprocess,sys,tempfile
from pathlib import Path
P=Path(__file__).resolve().parent.parent
W=P.parent
H=W/'ahc008_focus_solver/problems/ahc008'
sys.path.insert(0,str(H))
from benchmark_cases import run
T=W/'ahc008_focus_assets/tools/target/release/tester'
B=W/'ahc008_v24_standby_early_seam/solve'
C=P/'solve'
f=P/'fresh100.csv'
ref={(q['case'],q['model']):q for q in csv.DictReader(f.open())}
root=P/'flip_audit/tapes'
x=subprocess.check_output([sys.executable,str(P/'flip_audit/audit.py'),str(f),'--select'],text=True)
for line in x.splitlines():
 case, *_=line.split()
 item=P/'fresh-m5-100/in'/case
 raw=item.read_bytes()
 for arm,exe in [('baseline',B),('article',C)]:
  target=root/arm; target.mkdir(parents=True,exist_ok=True)
  a=target/(item.stem+'.actions'); tr=target/(item.stem+'.trace')
  refrow=ref[case,arm]; assert hashlib.sha256(raw).hexdigest()==refrow['input_sha256']
  with tempfile.TemporaryDirectory() as q:
   q=Path(q)
   labels='ARTICLE RESCUE BRIDGE EARLY_EXACT TERRY BANK'.split()
   args=[str(T),'env',*(f'AHC008_{k}_LOG={q/k}' for k in labels),str(exe)]
   for trial in range(5):
    data,e,rc,_=run(args,raw,4.5)
    assert rc==0
    if hashlib.sha256(data).hexdigest()==refrow['actions_sha256']:break
   else:
    print('unreproduced due to clock-sensitive trajectory',case,arm,flush=True)
    continue
  assert e.decode().strip()==f"Score = {refrow['score']}"
  a.write_bytes(data)
  b,er,cr,_=run([str(T),sys.executable,H/'benchmark_cases.py','--playback',a,tr],raw,12.)
  assert (b,er,cr)==(data,e,0)
 print(case,flush=True)
