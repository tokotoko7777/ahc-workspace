import csv,statistics as s,sys
old={r['case']:r for r in csv.DictReader(open(sys.argv[2])) if r['model']=='article'}
rows=list(csv.DictReader(open(sys.argv[1])))
assert len(rows)==2*len(old)==60
low=[];high=0
for a,b in zip(rows[::2],rows[1::2]):
 assert a['case']==b['case'] and a['input_sha256']==b['input_sha256']
 assert all(old[a['case']][k]==a[k] for k in ('actions_sha256','input_sha256','score'))
 m=int(a['humans'])
 if m>6:
  assert a['actions_sha256']==b['actions_sha256'] and not b['contract']
  high+=1
  continue
 header,*logs=[list(map(int,x.split())) for x in b['contract'].split(';')]
 assert len(logs)==header[1]==m and all(len(q)==7 for q in logs)
 assert sum(r[0] for r in logs)==112
 assert all(not r[3] or r[0]>0 for r in logs)
 low.append((header[0],logs))
 print(b['case'],header[0],[(n,m,b,s,e-1,g,k) for n,m,b,s,e,g,k in logs])
print('full identities of high workers',high)
print('routed cases',len(low),'makespan min/median/max',min(t for t,r in low),s.median(t for t,r in low),max(t for t,r in low))
j=[a for t,r in low for a in r]
print('jobs and crews',len(j),sum(r[0]>0 for r in j),sum(r[1]+r[2]>0 for r in j),'moves, builds, skipped, egress, egress walks',sum(a[1] for a in j),sum(a[2] for a in j),sum(a[3] for a in j),sum(a[4]>0 for a in j),sum(a[5] for a in j))
print('median full traversal time among exited',s.median(r[4]-1 for r in j if r[4]>0))
