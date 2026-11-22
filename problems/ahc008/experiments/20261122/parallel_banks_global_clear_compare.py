import csv,gzip,statistics as st,sys
from collections import Counter
if len(sys.argv) not in (2,3):
    raise SystemExit('Usage: compare_clear_frozen.py paired.csv [previous-frozen.csv.gz]')
with open(sys.argv[1]) as f: rows=list(csv.DictReader(f))
assert rows
if len(sys.argv)==3:
    with gzip.open(sys.argv[2],'rt') as f:
        ref={(r['case'],r['input_sha256']):r for r in csv.DictReader(f) if r['model']=='narrow_rescue'}
    assert len(rows)==len(ref)==100
    for r in rows:
        old=ref[r['case'],r['input_sha256']]
        assert old['actions_sha256']==r['baseline_actions_sha256']
        assert int(old['score'])==int(r['baseline'])
zeros=[r for r in rows if int(r['stop_turn'])>=0]
diff=Counter('win' if int(r['delta'])>0 else 'tie' if int(r['delta'])==0 else 'loss' for r in rows)
assert not diff['loss']
base=sum(int(r['baseline']) for r in rows)
score=sum(int(r['candidate']) for r in rows)
print(f'{len(rows)} cases, exact baseline {base:,} -> {score:,} (+{score-base:,}), {diff}')
print(f'{len(zeros)} clear early; {sum(int(r["baseline_walls_after_stop"]) for r in rows)} extra old walls')
for r in rows:
    if int(r['delta']):print(r['case'],r['stop_turn'],r['baseline'],r['candidate'],r['baseline_walls_after_stop'])
old=[float(r['baseline_wall_seconds']) for r in rows]
new=[float(r['candidate_wall_seconds']) for r in rows]
print('median, max wall:',st.median(old),st.median(new),max(old),max(new))
