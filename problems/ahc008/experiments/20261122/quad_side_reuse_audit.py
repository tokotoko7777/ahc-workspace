"""Early existing tactical wall segments that can supply a four-cell shell side."""
import argparse,csv,itertools,re,sys,tempfile
from pathlib import Path
from collections import Counter
from guarded_entry_trace import observed,STEPS,step
from stochastic_lock_audit import ADJ,ALL,B,V,idx,flood,wave,cells
BASE={'line':[(0,0),(0,1),(0,2),(0,3)],
      'square':[(0,0),(0,1),(1,0),(1,1)],
      'ell':[(0,0),(1,0),(2,0),(2,1)]}
def build_catalog(family):
    seen={}
    for swap,rx,ry in itertools.product((False,True),(-1,1),(-1,1)):
        xy=[(rx*(y if swap else x),ry*(x if swap else y)) for x,y in BASE[family]]
        a=min(x for x,y in xy);b=min(y for x,y in xy)
        xy=[(x-a,y-b) for x,y in xy]
        a=max(x for x,y in xy);b=max(y for x,y in xy)
        for r in range(30-a):
            for c in range(30-b):
                if r and c and r+a<29 and c+b<29:continue
                body=sum(B[30*(x+r)+y+c] for x,y in xy);shell=wave(body)&~body
                side=[]
                for i in (r-1,r+a+1):
                    if 0<=i<30:
                        u=sum(B[i*30+j] for j in range(c,c+b+1))
                        if u.bit_count()>=2 and u&shell==u:side.append(u)
                for j in (c-1,c+b+1):
                    if 0<=j<30:
                        u=sum(B[i*30+j] for i in range(r,r+a+1))
                        if u.bit_count()>=2 and u&shell==u:side.append(u)
                if side:seen[body]=(shell,side)
    return seen
def test(geo,hist,trace):
    out=Counter()
    for t in range(68):
        p,h,w=hist[t]
        old=sum(B[idx(x)] for x in w)
        humans=sum({B[idx(x)] for x in h})
        pets=sum({B[idx(x)] for x in p})
        shared=flood(humans,ALL^old)
        planned=0
        for j,c in enumerate(trace[2*t]):
            if c.islower():planned|=B[idx(step(h[j],STEPS[c.upper()]))]
        for body,(shell,sides) in geo.items():
            if (body|shell)&humans or body&old or not body&shared:continue
            built=any(s&old==s for s in sides)
            now=old|planned
            synchronous=any(s&now==s for s in sides)
            if not synchronous:continue
            area=1 if body&pets else 0
            gap=(shell&~now).bit_count()
            live_now=shell&~old
            if built:out['built_any']+=1
            else:out['needs_current_tactical_wall']+=1
            if area:out['occupied_built' if built else 'occupied_sync']+=1
            if not area or not 1<=gap<=3:continue
            out['occupied_gaps1to3']+=1
            outside=(ALL^old)&~(body|shell)
            reachable=humans
            for _ in range(2):reachable|=wave(reachable)&outside
            acts=wave(reachable)
            if (shell&~now)&acts != (shell&~now):continue
            out['occupied_all_remaining_stances2']+=1
            # At least one currently staffed pending gate with legal animal clearance.
            prohibited=pets|wave(pets)|humans
            if live_now&~prohibited&wave(humans):out['occupied_staffed_legal_wall']+=1
    return out
def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','solver','cases'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester,solver=a.tester.resolve(),a.solver.resolve()
    playback=a.harness.resolve()/'benchmark_cases.py'
    geo={f:build_catalog(f) for f in BASE}
    print({i:len(k) for i,k in geo.items()},file=sys.stderr)
    cols=('built_any','needs_current_tactical_wall','occupied_built','occupied_sync','occupied_gaps1to3',
          'occupied_all_remaining_stances2','occupied_staffed_legal_wall')
    out=csv.writer(sys.stdout,lineterminator='\n');out.writerow(['case','shape',*cols])
    for case in sorted(a.cases.glob('*.txt')):
        payload=case.read_bytes()
        cmd,err,rc,_=run([tester,solver],payload,3.)
        match=re.fullmatch(rb'Score = (\d+)\s*',err)
        assert rc==0 and match
        with tempfile.TemporaryDirectory() as d:
            path=Path(d)/'actions';path.write_bytes(cmd)
            trace=Path(d)/'trace'
            response,e,c,_=run([tester,sys.executable,playback,'--playback',path,trace],payload,10.)
            assert c==0 and response==cmd and e==err
            data=trace.read_text().splitlines()
        score,*_=replay(payload.decode(),data,cmd.decode().splitlines(),detailed=True)
        assert score==int(match[1])
        _,hist,_=observed(payload,data)
        for i,g in geo.items():
            counts=test(g,hist,data)
            out.writerow([case.name,i,*[counts[k] for k in cols]])
        sys.stdout.flush()
if __name__=='__main__':main()
