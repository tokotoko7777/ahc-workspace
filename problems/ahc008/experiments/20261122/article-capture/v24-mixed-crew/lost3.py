"""Diagnostic replay of the three held M=5 cases whose full clear disappeared."""
from pathlib import Path
from collections import Counter
import csv,hashlib,json,sys,tempfile
W=Path.cwd(); H=W/'ahc008_focus_solver/problems/ahc008'
sys.path[:0]=[str(H),str(H/'experiments/20261122')]
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,flood
from benchmark import replay
from benchmark_cases import run
t=W/'ahc008_focus_assets/tools/target/release/tester'
p=W/'ahc008_v24_split_crew'
r=list(csv.DictReader((p/'held100.csv').open()))
R={(x['case'],x['model']):x for x in r}
pair=[('baseline',W/'ahc008_terry_same_geometry/hybrid/source-release/frozen'),('article',p/'solve')]
indir=W/'ahc008_v24_five_portfolio/independent-seed3/in'
def state(frame):
 ps,hs,ws=frame
 root=flood(sum(B[k] for k in {idx(h) for h in hs}),ALL^sum(B[idx(w)] for w in ws))
 return ([i for i,p in enumerate(ps) if root&B[idx(p)]],root.bit_count(),len(ws))
def one(f,exe,model):
 raw=f.read_bytes(); known=R[(f.name,model)]
 with tempfile.TemporaryDirectory() as d:
  p=Path(d); z=p/'trace'; a=p/'actions'; g=p/'phase'
  act,err,rc,_=run([str(t),'env','AHC008_ARTICLE_LOG='+str(g),str(exe)],raw,4.5)
  assert rc==0 and hashlib.sha256(act).hexdigest()==known['actions_sha256'],(f,model)
  a.write_bytes(act); actions=act.decode().splitlines()
  b,e,c,_=run([str(t),sys.executable,H/'benchmark_cases.py','--playback',a,z],raw,12.)
  assert (b,e,c)==(act,err,0)
  trace=z.read_text().splitlines()
  assert replay(raw.decode(),trace,actions,detailed=True)[0]==int(known['score'])
  kinds,hist,_=observed(raw,trace)
  labels={int(x[0]):int(x[1]) for l in g.read_text().splitlines() if (x:=l.split())}
 shadows=[state(h) for h in hist]
 cuts=[];prev=set(shadows[0][0])
 for at,(accessible,area,walls) in enumerate(shadows[1:],1):
  new=prev-set(accessible)
  if new:
   pet_xy=[hist[at-1][0][i] for i in sorted(new)]
   cuts.append(dict(at=at,new=[(i,kinds[i],xy) for i,xy in zip(sorted(new),pet_xy)],area=area,remaining=len(accessible),mode=labels.get(at-1,0),builds=sum(c.islower() for c in actions[at-1])))
  prev=set(accessible)
 snapshots={at:{'live':shadows[at][0],'area':shadows[at][1],'walls':shadows[at][2],
                'builds_since_release':sum(c.islower() for line in actions[160:at] for c in line)}
            for at in [160,180,200,225,250,275,299,300]}
 return dict(score=int(known['score']),kinds=kinds,final=shadows[-1][0],cuts=cuts,
             snapshots=snapshots,mixed_turns=[x for x in labels if labels[x]==10])
out={}
for f in sorted(indir.glob('*.txt')):
 if R[(f.name,'baseline')]['all_clear']!='1' or R[(f.name,'article')]['all_clear']!='0':continue
 out[f.name]={k:one(f,e,k) for k,e in pair}
 for arm,s in out[f.name].items():
  print(f.name,arm,s['final'],s['mixed_turns'],[(t,len(v['live']),v['area'],v['builds_since_release']) for t,v in s['snapshots'].items()])
assert len(out)==3
Path(__file__).with_name('lost-three.json').write_text(json.dumps(out,indent=2)+'\n')
