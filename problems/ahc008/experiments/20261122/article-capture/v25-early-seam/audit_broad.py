"""Checks archived broad AHC008 rows against actual replays' input digests."""
import csv, gzip, hashlib, sys
from pathlib import Path
f=Path(sys.argv[1]); cases=Path(sys.argv[2])
op=gzip.open if f.suffix=='.gz' else open
with op(f,'rt') as r: rows=list(csv.DictReader(r))
inputs={x.name:hashlib.sha256(x.read_bytes()).hexdigest() for x in cases.glob('*.txt')}
assert len(rows)==200 and len(inputs)==100
assert {x['case'] for x in rows}==set(inputs)
assert len({(x['case'],x['model']) for x in rows})==200
for x in rows: assert inputs[x['case']]==x['input_sha256']
print(f,len(rows),'verified input digests')
