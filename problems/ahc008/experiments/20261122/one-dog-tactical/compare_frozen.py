import csv,hashlib,os,re,subprocess,sys,tempfile,time
from pathlib import Path
W=Path(__file__).resolve().parents[2]
PROTOCOL=Path(os.environ.get('AHC008_RELAY',W/'capture-first-analysis'/'benchmark.py')).resolve()
sys.path.insert(0,str(PROTOCOL.parent));from benchmark import replay
T=Path(os.environ.get('AHC008_TESTER',W/'ahc008_focus_assets'/'tools'/'target'/'release'/'tester')).resolve()
DIR=dict(zip('UDLR',((-1,0),(1,0),(0,-1),(0,1))))
def step(p,k):
 a,b=DIR[k.upper()];return p[0]+a,p[1]+b
def valid(p):return 1<=p[0]<=30 and 1<=p[1]<=30
def stats(text,stream):
 it=iter(text.splitlines()); n=int(next(it)); pets=[tuple(map(int,next(it).split())) for _ in range(n)]
 ps=[u[:2] for u in pets];m=int(next(it));hs=[tuple(map(int,next(it).split())) for _ in range(m)]
 w=set(); units=[0,0,0]
 for act,obs in zip(stream[::2],stream[1::2],strict=True):
  for i,c in enumerate(act):
   units[0 if c=='.' else 1 if c.isupper() else 2]+=1
   if c=='.':continue
   q=step(hs[i],c)
   if c.islower():w.add(q)
   else:hs[i]=q
  for j,s in enumerate(obs.split()):
   for c in s:
    if c!='.':ps[j]=step(ps[j],c)
 label={}
 for r in range(1,31):
  for c in range(1,31):
   q=(r,c)
   if q in label or q in w:continue
   label[q]=q;todo=[q]
   for p in todo:
    for k in DIR:
     z=step(p,k)
     if valid(z) and z not in w and z not in label:
      label[z]=q;todo.append(z)
 from collections import Counter
 area=Counter(label.values()); h=Counter(label[p]for p in hs); pcount=Counter(label[p]for p in ps)
 dist=[0]*5
 exact=[0]*5
 for i,q in enumerate(ps):
  r=label[q];k=pets[i][2]-1
  if h[r]:dist[k]+=1
  elif area[r]==4:exact[k]+=1
 safe={r:a for r,a in area.items()if h[r] and not pcount[r]}
 return [sum(x[2]==4 for x in pets), sum(h[x] for x in safe),sum(a*h[x] for x,a in safe.items()),sum(safe.values()),min(area[label[x]] if label[x] in safe else 0 for x in hs),*dist,*exact,*units]
keys='dogs clear_humans human_safe_area_sum distinct_safe_area min_safe_area residual_cow residual_pig residual_rabbit residual_dog residual_cat exact4_cow exact4_pig exact4_rabbit exact4_dog exact4_cat wait walk build'.split()
diagcols='mode prefix prefix_wall prefix_guard prefix_build prefix_walk prefix_wait prefix_exact_dog prefix_exact_other'.split()
with tempfile.TemporaryDirectory() as d,open(sys.argv[2],'x') as f:
 t=Path(d)/'t'
 csvout=csv.writer(f);csvout.writerow(['case','solver','score','action_sha256','direct_wall_sec']+keys+diagcols)
 for j,p in enumerate(sorted(Path(sys.argv[1]).glob('*.txt'))):
  inp=p.read_bytes()
  for arm in (['original','edge'] if not(j%2) else ['edge','original']):
   exe=str(Path('./tactical_'+arm).resolve())
   start=time.monotonic()
   direct=subprocess.run([T,exe],input=inp,capture_output=True,timeout=4,check=True,env=dict(os.environ,TRANSIT_DIAG='1'))
   wall=time.monotonic()-start
   x=subprocess.run([T,sys.executable,PROTOCOL,'--relay',exe,t],input=inp,capture_output=True,timeout=7,check=True,env=dict(os.environ,TRANSIT_DIAG='1'))
   assert direct.stdout==x.stdout and direct.stderr==x.stderr,(p,arm,direct.stderr,x.stderr)
   score=re.search(rb'^Score = (\d+)$',x.stderr,re.M)
   diag=re.search(rb'^TRANSIT ([0-9 ]+)$',x.stderr,re.M)
   assert score and len(x.stdout.splitlines())==300 and bool(diag)==(arm=='edge'),(p,arm,x.stderr)
   stream=t.read_text().splitlines()
   q=replay(inp.decode(),stream,x.stdout.decode().splitlines())
   assert int(score[1])==q[0]
   vals=stats(inp.decode(),stream)
   csvout.writerow([p.name,arm,int(score[1]),hashlib.sha256(x.stdout).hexdigest(),wall,*vals,*(map(int,diag[1].split())if diag else ['']*len(diagcols))])
   f.flush()
