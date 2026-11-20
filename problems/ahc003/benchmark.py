#!/usr/bin/env python3
import argparse, csv, hashlib, json, math, re, subprocess, time
from pathlib import Path

N = 30
Q = 1000

def independent(case, paths):
    rows = case.read_text().splitlines()
    contents = [line.split() for line in rows]
    h = [list(map(int, r)) for r in contents[:N]]
    v = [list(map(int, r)) for r in contents[N:2*N-1]]
    queries = contents[2*N-1:]
    if len(paths) != Q or len(queries) != Q:
        raise ValueError('wrong query count')
    total = 0.0
    for t, (s, path) in enumerate(zip(queries,paths,strict=True)):
        x, y, gx, gy, shortest = map(int,s[:5])
        visited = {(x,y)}
        length = 0
        for direction in path:
            if direction == 'U' and x > 0:
                x-=1; length+=v[x][y]
            elif direction == 'D' and x+1 < N:
                length+=v[x][y]; x+=1
            elif direction == 'L' and y > 0:
                y-=1; length+=h[x][y]
            elif direction == 'R' and y+1 < N:
                length+=h[x][y]; y+=1
            else:
                raise ValueError(f'query {t+1}: invalid direction / out of bounds')
            if (x,y) in visited:
                raise ValueError(f'query {t+1}: revisited vertex')
            visited.add((x,y))
        if (x,y)!=(gx,gy) or length < shortest or length<=0:
            raise ValueError(f'query {t+1}: invalid endpoint/distance')
        total = total * .998 + shortest/length
    return math.floor(total * 2312311 + .5)

def run_one(tester, binary, case, out, label):
    checkpoint = out/f'{case.stem}-{label}.json'
    if checkpoint.exists():
        return json.loads(checkpoint.read_text())
    result = {'case':case.stem,'solver':label,'score':0,'status':'failure'}
    start = time.perf_counter()
    try:
        p = subprocess.run([str(tester),str(case),str(binary)], capture_output=True,
                           text=True, check=True, timeout=2.0)
        wall = time.perf_counter()-start
        scores = re.findall(r'^Score = (\d+)$',p.stderr,re.MULTILINE)
        telemetry = next((json.loads(line) for line in p.stderr.splitlines() if line.startswith('{')), {})
        result.update(wall_s=wall,cpu_s=telemetry.get('cpu_seconds'))
        if wall>2.0 or len(scores)!=1:
            raise ValueError('timeout or unexpected official scores')
        score = independent(case,p.stdout.splitlines())
        if score != int(scores[0]) or score==0:
            raise ValueError('official/independent mismatch or illegal route')
        result.update(score=score,status='ok')
        (out/f'{case.stem}-{label}.routes').write_text(p.stdout)
        (out/f'{case.stem}-{label}.stderr').write_text(p.stderr)
    except (ValueError,subprocess.SubprocessError) as e:
        result.update(wall_s=time.perf_counter()-start,error=str(e))
    temp = checkpoint.with_suffix('.tmp')
    temp.write_text(json.dumps(result))
    temp.replace(checkpoint)
    return result

def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def main():
    p = argparse.ArgumentParser()
    for field in ('inputs','output','tester','baseline'):
        p.add_argument(field,type=Path)
    p.add_argument('probe',nargs='?',type=Path)
    p.add_argument('--label',default='baseline',choices=['baseline','probe'])
    a=p.parse_args()
    inputs=sorted(a.inputs.resolve().glob('*.txt'))
    if not inputs:
        p.error('no input files')
    tester,out=a.tester.resolve(),a.output.resolve()
    binaries=[(a.label,a.baseline.resolve())]
    if a.probe:
        if a.label != 'baseline': p.error('--label only applies to single-solver runs')
        binaries.append(('probe',a.probe.resolve()))
    out.mkdir(parents=True,exist_ok=True)
    meta={'tester':sha256(tester),'solvers':{k:sha256(v) for k,v in binaries},
          'inputs':{x.name:sha256(x) for x in inputs}}
    manifest=out/'manifest.json'
    if manifest.exists() and json.loads(manifest.read_text())!=meta:
        p.error('manifest mismatch')
    manifest.write_text(json.dumps(meta,indent=2)+'\n')
    results=[]
    for i,case in enumerate(inputs):
        for label,binary in (list(reversed(binaries)) if i%2 else binaries):
            results.append(run_one(tester,binary,case,out,label))
    with (out/'results.csv').open('w',newline='') as fp:
        w=csv.DictWriter(fp,fieldnames=['case','solver','score','status','wall_s','cpu_s','error'])
        w.writeheader()
        w.writerows(results)
    summary={}
    for label,_ in binaries:
        rows=[r for r in results if r['solver']==label]
        summary[label]={'cases':len(rows),'valid':sum(r['status']=='ok' for r in rows),
                        'total':sum(r['score'] for r in rows),
                        'max_wall_s':max(r['wall_s'] for r in rows),
                        'max_cpu_s':max((r.get('cpu_s') or 0 for r in rows),default=0)}
    (out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print(json.dumps(summary))
if __name__=='__main__':
    main()
