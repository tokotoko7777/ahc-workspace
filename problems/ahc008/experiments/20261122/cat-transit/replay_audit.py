"""Official replay archive reader. No contestant or analyzer sees the seed."""
import argparse,csv,re,subprocess,sys,tempfile
from pathlib import Path
p=argparse.ArgumentParser()
for x in ('tester','relay','cases','audit','output'):p.add_argument(x,type=Path)
p.add_argument('solver',nargs='+',help='ARM=BIN')
p.add_argument('--count',type=int,default=30)
a=p.parse_args()
sys.path.insert(0,str(a.relay.resolve().parent));from benchmark import replay
models={x.split('=',1)[0]:str(Path(x.split('=',1)[1]).resolve()) for x in a.solver}
cols='cat supported_inside near staffed_inside one_step_inside legal compact_ever compact_terminal p25 p50 p75 p25_staffed p25_one_step p25_legal p25_then_window p_max first_legal first_p25 first_inside'.split()
geo_cols='geometry_turns staffed_turns one_step_turns legal_turns'.split()
with tempfile.TemporaryDirectory() as d,open(str(a.output)+'-cats.csv','x') as out,open(str(a.output)+'-geometry.csv','x') as geo:
 trace=Path(d)/'trace.txt'
 cw=csv.writer(out);gw=csv.writer(geo)
 cw.writerow(['case','arm']+cols);gw.writerow(['case','arm','score']+geo_cols)
 for case in sorted(a.cases.glob('*.txt'))[:a.count]:
  inp=case.read_bytes()
  for arm,exe in models.items():
   raw=subprocess.run([a.tester.resolve(),sys.executable,a.relay.resolve(),'--relay',exe,trace],input=inp,capture_output=True,timeout=6,check=True)
   score=int(re.fullmatch(rb'Score = (\d+)\s*',raw.stderr)[1])
   move=trace.read_text().splitlines()
   ans=replay(inp.decode(),move,raw.stdout.decode().splitlines())
   assert score==(ans[0] if isinstance(ans,tuple) else ans)
   rows=inp.decode().splitlines()
   n=int(rows[0]);m=int(rows[n+1]);visible='\n'.join(rows[:n+m+2])+'\n'
   z=subprocess.run([str(a.audit.resolve())],input=(visible+'\n'.join(move)+'\n').encode(),capture_output=True,timeout=30,check=True)
   desc,*details=z.stdout.decode().splitlines()
   gw.writerow([case.name,arm,score,*desc.split(',')[1:]])
   for line in details:
    f=line.split(',');assert len(f)==len(cols)
    cw.writerow([case.name,arm,*f])
