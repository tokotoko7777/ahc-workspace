import csv,hashlib,re,os,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
H=Path(os.environ.get('AHC008_HARNESS',ROOT/'ahc008_focus_solver'/'problems'/'ahc008')).resolve()
A=ROOT/'ahc008_focus_assets'
sys.path[:0]=[str(Path(__file__).resolve().parent),str(H),str(H/'experiments'/'20261122')]
from benchmark_cases import run
from benchmark import replay
from guarded_entry_trace import observed
from parallel_banks_direction_bench import count as countbank

T=Path(os.environ.get('AHC008_TESTER',A/'tools'/'target'/'release'/'tester')).resolve()
base=Path(os.environ.get('AHC008_TACTICAL',A/'lock-tactical')).resolve()
plain=Path(os.environ.get('AHC008_PLAIN',A/'parallel-banks-direction')).resolve()
portfolio=Path('portfolio').resolve()
solvers={'tactical':base,'portfolio':portfolio}
def eligible(text):
 row=text.decode().splitlines()
 n=int(row[0]);m=int(row[n+1])
 return n,m,m>=8 and n<=12
files=sorted(Path('in').glob('*.txt'));assert len(files)==50
first=next((p for p in files if eligible(p.read_bytes())[2]),None)
if first is not None:
 d=first.read_bytes()
 r=run([T,plain,'5','0'],d,5.)
 q=run([T,portfolio],d,5.)
 assert r[:3]==q[:3] and r[2]==0,(first,r[1],q[1])
 Path('plain-golden.txt').write_text(first.name+' '+hashlib.sha256(r[0]).hexdigest()+' '+r[1].decode())
with open('fresh50.csv','x') as f:
 w=csv.DictWriter(f,fieldnames=('case','model','n','m','use_bank','score','sec','actions_sha256',
              'safe_humans','distinct_safe_area','all_clear',
              'residual','final4','bank_capture4','bank_cow','bank_pig','bank_rabbit','bank_dog','bank_cat'))
 w.writeheader()
 files=sorted(Path('in').glob('*.txt'));assert len(files)==50
 for i,p in enumerate(files):
  data=p.read_bytes();n,m,selected=eligible(data)
  hashes={}
  for label in ['tactical','portfolio'][::1 if i%2==0 else -1]:
   with tempfile.TemporaryDirectory() as d:
    d=Path(d);meta=d/'meta'; trace=d/'trace';acts=d/'acts'
    raw,stderr,rc,time=run([T,'env',f'AHC008_BANK_LOG={meta}',solvers[label]],data,5.)
    sc=re.fullmatch(rb'Score = (\d+)\s*',stderr)
    assert rc==0 and sc and len(raw.splitlines())==300,(p,label,stderr)
    acts.write_bytes(raw)
    x,y,rc2,_=run([T,sys.executable,H/'benchmark_cases.py','--playback',acts,trace],data,8.)
    assert (x,y,rc2)==(raw,stderr,0)
    tr=trace.read_text().splitlines();components=[]
    rr=replay(data.decode(),tr,raw.decode().splitlines(),detailed=True,final_components=components.extend)
    assert rr[0]==int(sc[1]),(rr,sc)
    kinds,hist,_=observed(data,tr)
    z=countbank(hist,kinds,meta.read_text() if meta.exists() else '')
    safe=[(h,a)for h,a,p in components if h and not p]
    sha=hashlib.sha256(raw).hexdigest();hashes[label]=sha
    w.writerow(dict(case=p.name,model=label,n=n,m=m,use_bank=int(selected),score=int(sc[1]),sec=time,
         actions_sha256=sha,safe_humans=sum(h for h,a in safe),distinct_safe_area=sum(a for h,a in safe),
         all_clear=int(sum(h for h,a in safe)==m),residual=z['final_residual_pets'], final4=sum(z[f'final4_{i}']for i in range(1,6)),
         bank_capture4=z.get('bank_capture4',''),**{f'bank_{name}':z.get(f'bank_species_{i}','')for i,name in enumerate(('cow','pig','rabbit','dog','cat'),1)}))
    f.flush()
  if not selected:assert hashes['portfolio']==hashes['tactical'],(p,'gate bypass changed commands')
