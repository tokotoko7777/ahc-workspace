import csv,subprocess,re,sys,os,time
from pathlib import Path
T='../../ahc008_focus_assets/tools/target/release/tester'
read=['elapsed','wall_turns','guard_turns','all_staffed_turns','builds','walks','waits','captured_dogs','captured_other']
with open(sys.argv[2],'x') as f:
 w=csv.DictWriter(f,['case','mode','dogs','pets','score','seconds']+read);w.writeheader()
 for k,p in enumerate(sorted(Path(sys.argv[1]).glob('*.txt'))[:int(sys.argv[3])]):
  inp=p.read_bytes(); h=inp.decode().splitlines()
  n=int(h[0]);dogs=sum(line.split()[-1]=='4' for line in h[1:n+1]);m=int(h[n+1])
  models=sys.argv[4:]
  for mode in models[k%len(models):]+models[:k%len(models)]:
   t=time.monotonic()
   out=subprocess.run([T,'./'+('cap4_original' if mode=='base' else 'transit_cap4')],input=inp,capture_output=True,timeout=4,env=dict(os.environ,TRANSIT_DIAG='1',TRANSIT_LIMIT=mode))
   sc=re.search(rb'^Score = (\d+)$',out.stderr,re.M); diag=re.search(rb'^TRANSIT ([0-9 ]+)$',out.stderr,re.M)
   if out.returncode!=0 or not sc or len(out.stdout.splitlines())!=300 or any(len(x)!=m for x in out.stdout.splitlines()):raise RuntimeError((p,mode,out.stderr))
   d=dict(case=p.name,mode=mode,pets=n,dogs=dogs,score=int(sc[1]),seconds=time.monotonic()-t)
   if mode!='base': d.update(dict(zip(read,map(int,diag[1].split()),strict=True)))
   w.writerow(d);f.flush()
