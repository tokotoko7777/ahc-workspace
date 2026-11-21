from pathlib import Path
import argparse,subprocess,re,os,hashlib,time,csv
ap=argparse.ArgumentParser()
ap.add_argument('tester',type=Path)
ap.add_argument('offset',type=int);ap.add_argument('count',type=int)
ap.add_argument('out',type=Path);ap.add_argument('model',nargs='+')
a=ap.parse_args()
with a.out.open('x') as f:
 w=csv.writer(f);w.writerow(('case','model','score','seconds','actions','selections','cuts','wins'))
 for i,p in enumerate(sorted(Path('in').glob('*.txt'))[a.offset:a.offset+a.count]):
  for name in a.model[i%len(a.model):]+a.model[:i%len(a.model)]:
   t=time.monotonic()
   command=[a.tester.resolve(),'./'+('control' if name=='original' else 'anchor')]
   if name.startswith('b'): command[-1]='./banchor'
   env=dict(os.environ,CAT_ANCHORS=name.removeprefix('b') if name!='original' else '0',CAT_ANCHOR_DIAG='1')
   q=subprocess.run(command,input=p.read_bytes(),capture_output=True,env=env,timeout=6)
   score=re.search(rb'^Score = (\d+)$',q.stderr,re.MULTILINE)
   diagnostics=re.search(rb'^ANCHORS (\d+) (\d+) (\d+)$',q.stderr,re.MULTILINE)
   assert score and q.returncode==0 and len(q.stdout.splitlines())==300,(p,name,q.stderr)
   w.writerow((p.name,name,int(score[1]),time.monotonic()-t,hashlib.sha256(q.stdout).hexdigest()[:16],*([int(x) for x in diagnostics.groups()] if diagnostics else [-1]*3)));f.flush()
