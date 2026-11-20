#!/usr/bin/env python3
"""Run the frozen solver on the official 1000-case AHC001 system suite."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import statistics
import subprocess
import tempfile
import threading
import time
from benchmark import check

def sha256(f):
    return hashlib.sha256(f.read_bytes()).hexdigest()

def publish(path, data):
    tmp=path.with_suffix(path.suffix+'.tmp')
    tmp.write_text(json.dumps(data,indent=2)+'\n')
    tmp.replace(path)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('cases',type=Path); ap.add_argument('solver',type=Path)
    ap.add_argument('vis',type=Path); ap.add_argument('seed_file',type=Path)
    ap.add_argument('out',type=Path)
    ap.add_argument('--jobs',type=int,default=1)
    ap.add_argument('--seconds',type=float,default=4.8)
    opts=ap.parse_args()
    if opts.jobs<1 or opts.seconds<=0 or opts.seconds>=5: ap.error('invalid time or worker count')
    if hashlib.md5(opts.seed_file.read_bytes()).hexdigest()!='8fc1ce3f4beabac6abc1bdb4206d7f7e':
        ap.error('wrong official seed file')
    paths=sorted(opts.cases.resolve().glob('*.txt'))
    if [p.name for p in paths]!=[f'{i:04d}.txt' for i in range(1000)]:
        ap.error('expected exact 1000 entries named 0000.txt..0999.txt')
    solver=opts.solver.resolve(); vis=opts.vis.resolve(); out=opts.out.resolve();out.mkdir(parents=True,exist_ok=True)
    manifest={'solver_sha256':sha256(solver),'scorer_sha256':sha256(vis),
              'seed_sha256':sha256(opts.seed_file),'input_sha256s':[sha256(f) for f in paths],
              'seconds_arg':opts.seconds,'wall_timeout_s':5.0}
    mf=out/'manifest.json'
    if mf.is_file():
        if json.loads(mf.read_text())!=manifest: raise SystemExit('checkpoint manifest differs from frozen run')
    else:
        publish(mf,manifest)
    lock=threading.Lock(); nw=len(list(out.glob('[0-9][0-9][0-9][0-9].json')))
    def run(f):
        nonlocal nw
        dest=out/(f.stem+'.json')
        if dest.is_file(): return
        raw=f.read_bytes()
        start=time.monotonic()
        try:
            q=subprocess.run([str(solver),str(opts.seconds)],input=raw,capture_output=True,timeout=5.0)
            elapsed=time.monotonic()-start
        except subprocess.TimeoutExpired:
            result={'case':f.stem,'score':0,'elapsed_s':time.monotonic()-start,'status':'timeout'}
        else:
            result={'case':f.stem,'score':0,'elapsed_s':elapsed,'status':'nonzero_exit'}
            (out/(f.stem+'.log')).write_bytes(q.stderr)
            target=out/(f.stem+'.out');target.write_bytes(q.stdout)
            if q.returncode==0:
                if elapsed>5: result['status']='timeout'
                else:
                    try: independent=check(raw,q.stdout)
                    except ValueError as e:
                        result.update(status='invalid',error=str(e))
                    else:
                        with tempfile.TemporaryDirectory() as cwd:
                            judge=subprocess.run([str(vis),str(f),str(target)],cwd=cwd,check=True,capture_output=True,text=True)
                        official=int(judge.stdout.strip())
                        if judge.stderr or official!=independent:
                            raise RuntimeError(f'judge mismatch on {f}: {official}, {independent}, {judge.stderr}')
                        result.update(status='ok',score=official)
        publish(dest,result)
        with lock:
            nw+=1
            if nw%10==0 or result['status']!='ok': print(nw,result,flush=True)
    with ThreadPoolExecutor(max_workers=opts.jobs) as pool:
        for _ in pool.map(run,paths): pass
    rows=[json.loads((out/(p.stem+'.json')).read_text()) for p in paths]
    final={'cases':len(rows),'valid':sum(r['status']=='ok' for r in rows),
           'sum_score':sum(r['score'] for r in rows),'winner_reference':993823071683,
           'max_elapsed_s':max(r['elapsed_s'] for r in rows),
           'median_elapsed_s':statistics.median(r['elapsed_s'] for r in rows)}
    publish(out/'summary.json',final)
    print(json.dumps(final,indent=2))
if __name__=='__main__':main()
