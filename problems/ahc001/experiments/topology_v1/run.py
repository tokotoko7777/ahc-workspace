from pathlib import Path
from statistics import mean,median
import argparse
import csv
import json
import subprocess
import sys
import time
import tempfile
source=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(source))
ap=argparse.ArgumentParser()
ap.add_argument('work',type=Path)
ap.add_argument('diagnostic',type=Path)
ap.add_argument('vis',type=Path)
args=ap.parse_args()
root=args.work.resolve()
from benchmark import check
work=root/'checked';work.mkdir(exist_ok=True)
files=sorted((root/'input/in').glob('*.txt'))
assert len(files)==8
vis=args.vis.resolve()
rows=[]
with tempfile.TemporaryDirectory() as temporary:
 for f in files:
  d=work/f.stem;d.mkdir(exist_ok=True)
  s=root/'baseline/v64'/f.name
  cases=[d/x for x in ('common.out','pure.out','topology.out')]
  start=time.monotonic()
  p=subprocess.run([str(args.diagnostic.resolve()),str(f),str(s),*(str(x) for x in cases)],
                  check=True,capture_output=True,text=True,timeout=2)
  elapsed=time.monotonic()-start
  a,b,c,u,tried,switches,pairs_won,regrown=map(int,p.stdout.split())
  for f2, expected in zip([s,*cases],[a,b,c,u],strict=True):
   q=subprocess.run([str(vis),str(f),str(f2)],cwd=temporary,check=True,
                    capture_output=True,text=True)
   indep=check(f.read_bytes(),f2.read_bytes())
   assert not q.stderr and expected==indep==int(q.stdout), (q.stderr,expected,indep,q.stdout)
  rows.append(dict(case=f.name,baseline=a,common=b,pure=c,topology=u,
      control_delta=c-b,topology_delta=u-b,delta_against_control=u-c,
      trial_pairs=tried,switched_constraints=switches,topology_pair_wins=pairs_won,
      blocker_regrown=regrown,wall_s=elapsed))
  print(rows[-1],flush=True)
with (root/'RESULTS.csv').open('w',newline='') as f:
 w=csv.DictWriter(f,fieldnames=rows[0],lineterminator='\n');w.writeheader();w.writerows(rows)
d=[x['delta_against_control'] for x in rows]
summ=dict(n=len(rows),wins=sum(x>0 for x in d),ties=sum(x==0 for x in d),
    mean_delta=mean(d),median_delta=median(d),
    mean_control_gain=mean(x['control_delta'] for x in rows),
    mean_topology_gain=mean(x['topology_delta'] for x in rows),
    tested_pairs=sum(x['trial_pairs'] for x in rows),
    changed_separation=sum(x['switched_constraints'] for x in rows),
    recovered_blocker=sum(x['blocker_regrown'] for x in rows),
    max_wall_s=max(x['wall_s'] for x in rows))
(root/'SUMMARY.json').write_text(json.dumps(summ,indent=2)+'\n')
print(summ)
