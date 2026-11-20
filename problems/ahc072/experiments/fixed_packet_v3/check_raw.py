"""Validate raw mixed itineraries rejected by the frozen-caravan fallback."""
import argparse
import csv
from pathlib import Path
import subprocess
import sys

sys.path.insert(0,str(Path(__file__).resolve().parents[2]))
from benchmark import replay

p=argparse.ArgumentParser(description=__doc__)
for name in ('vis','raw','work'):p.add_argument(name,type=Path)
a=p.parse_args()
vis,raw,work=a.vis.resolve(),a.raw.resolve(),a.work.resolve()
with (work/'results.csv').open() as f:
    expected=[x for x in csv.DictReader(f) if x['arm']=='packet']
assert len(expected)==32
for split in ('dev','holdout'):
    selected=[r for r in expected if r['split']==split]
    folder=work/split
    for i,row in enumerate(selected):
        inp=folder/'in'/f'{i:04}.txt'
        ans=folder/f'{i:04}-raw.txt'
        data=inp.read_bytes()
        run=subprocess.run([raw],input=data,capture_output=True,check=True,timeout=2)
        score,*_=replay(data,run.stdout)
        assert score==int(row['mixed'])
        ans.write_bytes(run.stdout)
        judge=subprocess.run([vis,inp,ans],cwd=folder,capture_output=True,text=True,check=True)
        assert judge.stdout.strip()==f'Score = {score}',judge.stdout
print('32 raw alternatives independently replayed, official scores exact')
