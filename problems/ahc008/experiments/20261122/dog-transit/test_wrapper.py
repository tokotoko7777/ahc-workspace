import subprocess,csv,re,sys,os,time
from pathlib import Path
T='../../ahc008_focus_assets/tools/target/release/tester'
read=['elapsed','wall_turns','guard_turns','all_staffed_turns','builds','walks','waits','captured_dogs','captured_other','residual','largest_dogs']
with open(sys.argv[2],'x') as f:
 w=csv.DictWriter(f,['case','dogs','pets','score','seconds']+read)
 w.writeheader()
 for p in sorted(Path(sys.argv[1]).glob('*.txt')):
  t=time.monotonic()
  inp=p.read_bytes()
  h=inp.decode().splitlines()
  n=int(h[0]);dogs=sum(line.split()[-1]=='4' for line in h[1:n+1]);m=int(h[n+1])
  out=subprocess.run([T,'./standalone'],input=inp,capture_output=True,timeout=4,env=dict(os.environ,TRANSIT_DIAG='1'))
  sc=re.search(rb'^Score = (\d+)$',out.stderr,re.M)
  diag=re.search(rb'^TRANSIT ([0-9 ]+)$',out.stderr,re.M)
  if out.returncode!=0 or not sc or not diag or len(out.stdout.splitlines())!=300 or any(len(x)!=m for x in out.stdout.splitlines()):raise RuntimeError((p,out.stderr))
  d=dict(case=p.name,pets=n,dogs=dogs,score=int(sc[1]),seconds=time.monotonic()-t)
  d.update(dict(zip(read,map(int,diag[1].split()),strict=True)))
  assert d['captured_dogs']+d['largest_dogs']==dogs
  assert d['captured_dogs']+d['captured_other']+d['residual']==n
  assert d['builds']+d['walks']+d['waits']==d['elapsed']*m
  w.writerow(d);f.flush()
