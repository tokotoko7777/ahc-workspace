"""Compare pairs against an earlier exact article v4 ledger."""
from collections import defaultdict
import csv, gzip, statistics as stats, sys

def read(fn):
    f=gzip.open if fn.endswith('.gz') else open
    with f(fn,'rt') as r:return list(csv.DictReader(r))

a,e=map(read,sys.argv[1:3])
ref={(r['case'],r['input_sha256']):r for r in e if r['model']=='article'}
groups=defaultdict(dict)
for r in a:groups[(r['case'],r['input_sha256'])][r['model']]=r
assert len(groups)==len(ref)==100 and set(groups)==set(ref)
def exact(q):return q['actions_sha256'],int(q['score'])
bt=[];pt=[];hit=queries=max_slots=score=0; clear=0
for k,arms in groups.items():
    a,b=arms['baseline'],arms['article']
    assert exact(a)==exact(b)==exact(ref[k]),k
    assert a['captures']==b['captures']==ref[k]['captures'],k
    bt.append(float(a['wall_sec']));pt.append(float(b['wall_sec']))
    hit+=int(b['passive_hits']);queries+=int(b['passive_calls']);max_slots=max(max_slots,int(b['passive_pool']))
    score+=int(b['score']); clear+=int(b['all_clear'])
print(f'{len(bt)} exact historic hashes, {clear} all clear, score {score:,}')
print(f'median {stats.median(bt):.6f}→{stats.median(pt):.6f}, max {max(bt):.6f}→{max(pt):.6f}')
print(f'paired median relative difference {stats.median((y/x-1)*100 for x,y in zip(bt,pt)):+.3f}%, faster {sum(x>y for x,y in zip(bt,pt))}/{len(bt)}')
print(f'cache hits {hit:,}/{queries:,}, max pool slots {max_slots}')
