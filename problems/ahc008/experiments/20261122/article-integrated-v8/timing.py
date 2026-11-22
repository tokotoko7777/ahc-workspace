"""Observed first low-residue transition times and pre-100 isolates."""
import csv,collections,sys,json,statistics
rows=list(csv.DictReader(open(sys.argv[1])))
pair=list(zip(rows[::2],rows[1::2]))
assert all(a['case']==b['case'] and a['model']=='baseline' for a,b in pair)
def metrics(r):
    c=collections.Counter(); early=collections.Counter(); early4=collections.Counter()
    for e in r['captures'].split(';'):
        if not e:continue
        t,p,k,a,irr=map(int,e.split(':'))
        c[t]+=1
        if t<=100:
            early[k]+=1
            if a==4:early4[k]+=1
    n=int(r['animals']);one=le2=301
    for t,g in sorted(c.items()):
        n-=g
        if n==1 and one==301:one=t
        if n<=2 and le2==301:le2=t
    assert n==int(r['final_reachable'])
    return one,le2,early,early4
r=[(metrics(a),metrics(b)) for a,b in pair]
def arm(k):
    a=[x[k] for x in r]
    one=[x[0] for x in a if x[0]<=300]
    le2=[x[1] for x in a if x[1]<=300]
    e=collections.Counter();s=collections.Counter()
    for x in a:e.update(x[2]);s.update(x[3])
    return dict(exact1_cases=len(one),exact1_median=statistics.median(one) if one else None,
                le2_cases=len(le2),le2_median=statistics.median(le2) if le2 else None,
                exact1_by260=sum(t<=260 for t in one),le2_by260=sum(t<=260 for t in le2),
                early_total=sum(e.values()),early4_total=sum(s.values()),early=e,early4=s)
out=dict(baseline=arm(0),variant=arm(1),
         one_paired=collections.Counter((b[0]>a[0])-(b[0]<a[0]) for a,b in r),
         le2_paired=collections.Counter((b[1]>a[1])-(b[1]<a[1]) for a,b in r))
print(json.dumps(out,indent=2))
