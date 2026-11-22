"""Validate official identity and summarize sequential paired tester timings.
Usage: compare_frozen.py NEW.csv[.gz] FROZEN.csv.gz
"""
import csv
import gzip
import statistics as st
import sys

def read(fn):
    op = gzip.open if fn.endswith('.gz') else open
    with op(fn,'rt',encoding='utf-8') as f:
        return list(csv.DictReader(f))

current, frozen = map(read,sys.argv[1:])
ref={(r['case'],r['input_sha256']): r for r in frozen if r['model']=='narrow_rescue'}
data={}
for row in current:
    data.setdefault((row['case'],row['input_sha256']),{})[row['model']]=row
assert len(data)==100 and set(data)==set(ref)
def key(r): return r['actions_sha256'], int(r['score'])
a=[]; b=[]; score=0; queries=hits=capacity=0
for k, rs in data.items():
    old, fast = rs['rescue3'], rs['narrow_rescue']
    assert key(ref[k])==key(old)==key(fast),k
    a.append(float(old['time'])); b.append(float(fast['time']))
    score+=int(fast['score'])
    queries+=int(fast['memo_queries']);hits+=int(fast['memo_hits'])
    capacity=max(capacity,int(fast['memo_capacity']))
print(f'{len(data)} identical; score={score:,}')
print(f'median {st.median(a):.6f} -> {st.median(b):.6f}; max {max(a):.6f} -> {max(b):.6f}')
print(f'paired median change {st.median((y/x-1)*100 for x,y in zip(a,b)):+.3f}%; faster {sum(y<x for x,y in zip(a,b))}')
print(f'cache hits {hits:,}/{queries:,} = {100*hits/queries:.3f}%; max component-capacity {capacity}')
