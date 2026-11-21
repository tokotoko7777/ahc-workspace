import argparse,csv,importlib.util,re,subprocess,tempfile,time
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('repo',type=Path);p.add_argument('inputs',type=Path);p.add_argument('vis',type=Path);p.add_argument('out',type=Path);p.add_argument('--entry',required=True,action='append');p.add_argument('--resume',action='store_true')
a=p.parse_args()
spec=importlib.util.spec_from_file_location('independent',a.repo/'problems/ahc006/benchmark.py')
mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
inputs=a.inputs.resolve();vis=a.vis.resolve();a.out=a.out.resolve();a.out.mkdir(parents=True,exist_ok=True)
assert sorted(x.name for x in inputs.glob('*.txt'))==[f'{i:04}.txt' for i in range(100)]
entries=[]
for e in a.entry:
 label,path=e.split('=',1); assert re.fullmatch(r'[A-Za-z0-9_]+',label)
 entries.append((label,Path(path).resolve()));(a.out/label).mkdir(exist_ok=True)
assert len(set(l for l,b in entries))==len(entries)
C=a.out/'results.csv'; fields=('case','solver','score','distance','wall_s','iterations')
done=set()
if a.resume:
 with C.open() as f:
  for row in csv.DictReader(f):
   assert set(row)==set(fields)
   k=(row['case'],row['solver'])
   assert k not in done
   done.add(k)
with tempfile.TemporaryDirectory() as tmp,C.open('a' if a.resume else 'x',newline='') as f:
 w=csv.writer(f,lineterminator='\n')
 if not a.resume:w.writerow(fields);f.flush()
 for i,p in enumerate(sorted(inputs.glob('*.txt'))):
  data=p.read_bytes()
  for label,exe in entries[i%len(entries):]+entries[:i%len(entries)]:
   if (p.stem,label) in done: continue
   start=time.monotonic()
   r=subprocess.run([exe],capture_output=True,input=data,check=True,timeout=2)
   wall=time.monotonic()-start
   assert wall<2,(i,label,wall)
   distance,score=mod.check(data,r.stdout)
   out=a.out/label/p.name
   out.write_bytes(r.stdout)
   (out.parent/(p.stem+'.log')).write_bytes(r.stderr)
   v=subprocess.run([vis,p,out],cwd=tmp,text=True,capture_output=True,check=True)
   assert v.stdout.strip()==f'Score = {score}' and not v.stderr,(i,label,score,v.stdout,v.stderr)
   z=re.search(rb'iterations=(\d+)',r.stderr)
   w.writerow((p.stem,label,score,distance,f'{wall:.6f}',int(z[1]) if z else ''));f.flush()
   print(p.stem,label,score,distance,f'{wall:.6f}',flush=True)
