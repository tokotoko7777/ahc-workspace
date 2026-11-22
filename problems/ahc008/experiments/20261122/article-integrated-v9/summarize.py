"""Aggregate physical construction/egress telemetry for paired dev."""
import csv,json,statistics,collections,sys
r=list(csv.DictReader(open(sys.argv[1])))
def one(rs):
    st=[json.loads(x['stages']) for x in rs]
    pile=lambda k:[z for x in st for z in x[k] if z>=0]
    ready=pile('ready');staff=pile('staff');caught=pile('first_lock')
    staff_gap=[b-a for x in st for a,b in zip(x['ready'],x['staff']) if a>=0 and b>=0]
    lock_gap=[b-a for x in st for a,b in zip(x['ready'],x['first_lock']) if a>=0 and b>=0]
    z=dict(ready_count=len(ready),ready_median=statistics.median(ready),staff_count=len(staff),
           staff_median=statistics.median(staff),staff_gap_median=statistics.median(staff_gap),
           lock_count=len(caught),lock_median=statistics.median(caught),lock_gap_median=statistics.median(lock_gap))
    z['milestones']={t:{k:sum(x['milestones'][t][k] for x in st) for k in st[0]['milestones'][t]} for t in ('50','75','100')}
    w=[x['waves'] for x in st if 'waves' in x]
    if w:
        rows=[k for q in w for k in q[2]]
        z['waves']=dict(nominal_median=statistics.median(q[0] for q in w),deadline_median=statistics.median(q[1] for q in w),
              status=collections.Counter(row[1] for row in rows),
              complete_median=statistics.median([x[0] for x in rows if x[1]==1]) if any(x[1]==1 for x in rows) else None)
    return z
print(json.dumps(dict(baseline=one(r[::2]),candidate=one(r[1::2])),indent=2))
