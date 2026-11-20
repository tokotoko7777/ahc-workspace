import argparse
import csv
import re
import subprocess
import tempfile
import time
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[2]))
from benchmark import score_case

parser=argparse.ArgumentParser()
parser.add_argument('inputs',type=Path)
parser.add_argument('starts',type=Path)
parser.add_argument('exe',type=Path)
parser.add_argument('vis',type=Path)
parser.add_argument('out',type=Path)
args=parser.parse_args()
inputs,starts,exe,vis,out=[x.resolve() for x in [args.inputs,args.starts,args.exe,args.vis,args.out]]
out.mkdir(parents=True,exist_ok=True)
rows=[]
with tempfile.TemporaryDirectory() as td:
    starts_time={x['case']:float(x['elapsed_s']) for x in csv.DictReader((starts/'results.csv').open())}
    for k,f in enumerate(sorted(inputs.glob('*.txt'))):
        inp=f.read_text()
        b=starts/'selected'/f.name
        baseline=score_case(inp,b.read_text())[0]
        names=['control','mixed']
        for mode in names[k%2:]+names[:k%2]:
            d=out/mode
            d.mkdir(exist_ok=True)
            t=time.monotonic()
            q=subprocess.run([exe,f,b,mode],capture_output=True,text=True,check=True,timeout=.19)
            elapsed=time.monotonic()-t
            score,hits,empty=score_case(inp,q.stdout)
            assert score>=baseline and starts_time[f.name]+elapsed<3.0
            p=d/f.name;p.write_text(q.stdout)
            (d/(f.stem+'.log')).write_text(q.stderr)
            judge=subprocess.run([vis,f,p],cwd=td,capture_output=True,text=True,check=True)
            match=re.fullmatch(r'Score = (\d+)\s*',judge.stdout)
            assert not judge.stderr and match and int(match.group(1))==score
            line_accept=int(re.search(r'line_accept=(\d+)',q.stderr).group(1))
            row=(f.name,mode,baseline,score,score-baseline,hits,empty,starts_time[f.name],elapsed,starts_time[f.name]+elapsed,line_accept)
            rows.append(row)
            print(*row,flush=True)
    assert rows
    with (out/'results.csv').open('w',newline='') as f:
        w=csv.writer(f,lineterminator='\n')
        w.writerow(['case','mode','initial_score','score','gain','covered','empty','start_time_s','post_time_s','solver_sum_s','accepted_line_moves'])
        w.writerows(rows)
