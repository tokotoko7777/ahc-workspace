import csv,json,statistics as s,sys
r=list(csv.DictReader(open(sys.argv[1])))
q=list(csv.DictReader(open(sys.argv[2])))
checks={x['case']:x['actions_sha256'] for x in q if x['model']=='article'}
assert len(checks)==len(r)//2 and all(checks[x['case']]==x['actions_sha256'] for x in r if x['model']=='baseline')
for g in [0,1,2]:
 b=[]
 for v,c in zip(r[::2],r[1::2]):
  n=int(v['humans'])
  if g==1 and n>6 or g==2 and n<=6: continue
  d=[list(map(int,w.split())) for w in c['demand'].split(';') if w]
  if not d:raise ValueError("missing evidence")
  bm=[list(map(int,w.split())) for w in v['telemetry'].split(';') if w]
  cm=[list(map(int,w.split())) for w in c['telemetry'].split(';') if w]
  times=[next((t for t,m,_,_ in mm if m==2),999) for mm in (bm,cm)]
  loss=[t for (t0,n0,r0,u0,f0,s0),(t,n,r,u,f,ss) in zip(d,d[1:]) if t==t0+1 and t<=100 and s0 and not ss]
  local_loss=[(t,k) for (t0,n0,r0,u0,f0,s0),(t,n,r,u,f,ss) in zip(d,d[1:]) if t==t0+1 and t<=100 for k in range(4) if f0>>k&1 and not f>>k&1]
  b.append((times,loss,local_loss,d,bm,cm))
 print(g,'cases',len(b),'first_full',*[s.median(x[0][i] for x in b) for i in (0,1)],'pre101_full_turns',*[sum(t<101 and m==2 for x in b for t,m,_,_ in x[4+i]) for i in (0,1)],'full_reversals',sum(len(x[1]) for x in b),'cases',sum(bool(x[1]) for x in b),'bank_reversals',sum(len(x[2]) for x in b),'cases',sum(bool(x[2]) for x in b))
 for t0 in (35,50,80,100,133,150):
  f=[(n,r) for x in b for t,n,r,*_ in x[3] if t==t0]
  if f: print(t0,len(f),s.median(x[0] for x in f),s.median(x[1] for x in f))
