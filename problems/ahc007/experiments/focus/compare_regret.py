import csv
from collections import defaultdict
import pathlib
import subprocess
import sys
import tempfile

inputs, previous, current, profiler, output=(pathlib.Path(x).resolve() for x in sys.argv[1:])
groups=defaultdict(lambda:defaultdict(int))
with tempfile.TemporaryDirectory(prefix="ahc007-regret-") as work:
  work=pathlib.Path(work)
  for case in sorted(inputs.glob('*.txt')):
    for tool in (previous,current):
      dest=work/"answer"
      with case.open("rb") as inp,dest.open("wb") as out:
        subprocess.run([tool],stdin=inp,stdout=out,capture_output=False,stderr=subprocess.DEVNULL,check=True)
      r=subprocess.run([profiler,case,dest],capture_output=True,check=True,text=True)
      for row in csv.reader(r.stdout.splitlines()):
        t,take,loss,k,alts=map(int,row)
        g=groups[(tool.name,t//250,take)]
        g['count']+=1;g['total']+=loss
        g['incident_only']+=loss if alts>0 else 0
with output.open("w") as f:
  w=csv.writer(f,lineterminator="\n")
  w.writerow(("variant","phase_250","taken","num_lost","true_regret","direct_alternative_regret"))
  for key,row in sorted(groups.items()):
    w.writerow((*key,row['count'],row['total'],row['incident_only']))
