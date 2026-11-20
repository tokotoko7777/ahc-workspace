import argparse
from pathlib import Path
import csv
import re
import subprocess
import tempfile
import time

def score_case(inp, out):
    n, m, *words = inp.split()
    n, m = int(n), int(m)
    board = out.split()
    if len(board)!=n or len(words)!=m or any(len(r)!=n or set(r)-set('ABCDEFGH.') for r in board):
        raise ValueError('input size or illegal output')
    columns=[''.join(s) for s in zip(*board,strict=True)]
    lines=[w + w[:n-1] for w in [*board,*columns]]
    found=sum(any(w in line for line in lines) for w in words)
    empty=sum(s.count('.') for s in board)
    x,y=(found,m) if found<m else (2*n*n,2*n*n-empty)
    return (2*(10**8)*x+y)//(2*y),found,empty

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('inputs',type=Path);ap.add_argument('out',type=Path)
    ap.add_argument('--vis',type=Path,required=True)
    ap.add_argument('--entry',required=True,action='append')
    ap.add_argument('--start',type=int,default=0);ap.add_argument('--end',type=int)
    a=ap.parse_args()
    entries=[(n,Path(p).resolve()) for n,p in (x.split('=',1) for x in a.entry)]
    vis=a.vis.resolve();d=a.out.resolve();d.mkdir(parents=True,exist_ok=True)
    cases=sorted(a.inputs.resolve().glob('*.txt'))[a.start:a.end]
    if not cases: raise ValueError('no cases')
    with tempfile.TemporaryDirectory() as run_dir, (d/'results.csv').open('w',newline='') as f:
        csv_out=csv.writer(f,lineterminator='\n')
        csv_out.writerow(('case','solver','score','covered','empty','elapsed_s'))
        for k,p in enumerate(cases):
            for label, exe in entries[k%len(entries):]+entries[:k%len(entries)]:
                dest=d/label;dest.mkdir(exist_ok=True)
                t=time.monotonic()
                result=subprocess.run([exe],input=p.read_bytes(),capture_output=True,check=True,timeout=3)
                elapsed=time.monotonic()-t
                case=dest/p.name;case.write_bytes(result.stdout)
                (dest/(p.stem+'.log')).write_bytes(result.stderr)
                own,covered,empty=score_case(p.read_text(),result.stdout.decode())
                visual=subprocess.run([vis,p,case],cwd=run_dir,capture_output=True,text=True,check=True)
                match = re.fullmatch(r'Score = (\d+)\s*',visual.stdout)
                if not match or visual.stderr or int(match.group(1))!=own:
                    raise RuntimeError(f'official score mismatch {p.name}: {visual.stdout!r} {visual.stderr!r} vs {own}')
                row=(p.name,label,own,covered,empty,f'{elapsed:.6f}')
                csv_out.writerow(row);f.flush();print(*row,flush=True)
if __name__=='__main__':main()
