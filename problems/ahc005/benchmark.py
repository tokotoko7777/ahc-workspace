import argparse,csv,re,subprocess,tempfile,time
from pathlib import Path

def score_walk(inp,out):
    n,sr,sc,*board=inp.split()
    n,sr,sc=int(n),int(sr),int(sc)
    route=out.strip()
    moves={'U':(-1,0),'D':(1,0),'L':(0,-1),'R':(0,1)}
    if set(route)-moves.keys(): raise ValueError('invalid move')
    seen=set();visited=set();r,c=sr,sc;cost=0
    def on(rr,cc):return 0<=rr<n and 0<=cc<n and board[rr][cc]!='#'
    def observe(r,c):
        if (r,c) in visited:return
        visited.add((r,c));seen.add((r,c))
        for a,b in moves.values():
            x,y=r+a,c+b
            while on(x,y):seen.add((x,y));x+=a;y+=b
    observe(r,c)
    for ch in route:
        a,b=moves[ch];r+=a;c+=b
        if not on(r,c):raise ValueError('step outside road')
        cost+=int(board[r][c]);observe(r,c)
    if (r,c)!=(sr,sc):raise ValueError('does not return')
    total=sum(ch!='#' for line in board for ch in line)
    if len(seen)!=total:raise ValueError('not fully covered')
    if cost<=0:raise ValueError('invalid travel cost')
    score=10000+(20000000*n+cost)//(2*cost)
    return score,cost,len(seen)

if __name__=='__main__':
    ap=argparse.ArgumentParser()
    ap.add_argument('inputs',type=Path);ap.add_argument('out',type=Path)
    ap.add_argument('--vis',type=Path,required=True);ap.add_argument('--entry',action='append',required=True)
    ap.add_argument('--start',type=int,default=0);ap.add_argument('--end',type=int)
    o=ap.parse_args()
    inputs=sorted(o.inputs.resolve().glob('*.txt'))[o.start:o.end]
    assert inputs
    entries=[(lab,Path(path).resolve()) for lab,path in (s.split('=',1) for s in o.entry)]
    vis=o.vis.resolve();out=o.out.resolve();out.mkdir(parents=True,exist_ok=True)
    with (out/'results.csv').open('w',newline='') as f,tempfile.TemporaryDirectory() as tmp:
        w=csv.writer(f,lineterminator='\n')
        w.writerow(('case','solver','score','travel','roads','elapsed_s','pre_splice_travel'))
        for k,p in enumerate(inputs):
            for label,exe in entries[k%len(entries):]+entries[:k%len(entries)]:
                d=out/label;d.mkdir(exist_ok=True)
                start=time.monotonic()
                r=subprocess.run([exe],input=p.read_text(),capture_output=True,text=True,timeout=3,check=True)
                elapsed=time.monotonic()-start
                score,cost,roads=score_walk(p.read_text(),r.stdout)
                target=d/p.name;target.write_text(r.stdout)
                (d/(p.stem+'.log')).write_text(r.stderr)
                q=subprocess.run([vis,p,target],cwd=tmp,capture_output=True,text=True,check=True)
                match=re.fullmatch(r'Score = (\d+)\s*',q.stdout)
                assert not q.stderr and match and int(match.group(1))==score,(q.stdout,q.stderr,score)
                saved=re.search(r'before_splice=(\d+)',r.stderr)
                v=(p.name,label,score,cost,roads,f'{elapsed:.6f}',int(saved.group(1)) if saved else cost)
                w.writerow(v);f.flush();print(*v,flush=True)
