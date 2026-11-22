"""Co-occurring compact cap candidates, joint two-worker/two-turn cost."""
import argparse,csv,itertools,re,sys,tempfile
from collections import defaultdict
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ADJ,ALL,B,idx,flood,wave,cells

def md(a,b):
    return abs(a//30-b//30)+abs(a%30-b%30)

def gatebs(gs):
    return sum(B[i] for i in gs)

def two_turn(opened,hs,gs,interior,blocked_at,baseline_actions=None):
    """Find exact least displaced jobs with observed per-turn gate blocks.

    If blocked_at is zero, only route/worker workload matter. Baseline moves
    of unassigned workers are frozen to their original actions (and excluded
    if they interfere), except in the route-only estimate. Pet trajectories
    are never resimulated by this diagnostic.
    """
    goal=set(gs)
    gmask=gatebs(gs)
    ext=opened&~(interior|gmask)
    best=None
    for pair in itertools.combinations(range(len(hs)),2):
        def move_go(p,step_t,closed):
            choices=[(p,'.',None)]
            for q in ADJ[p]:
                d=q-p
                j={-30:0,30:1,-1:2,1:3}[d]
                if B[q]&ext and not step_t:
                    if any(md(q,g)==1 for g in goal-closed):
                        choices.append((q,'UDLR'[j],None))
                if q in goal-closed and not B[q]&blocked_at[step_t]:
                    choices.append((p,'udlr'[j],q))
            return choices
        for a in itertools.product(*(move_go(hs[i],0,set()) for i in pair)):
            built={v for _,_,v in a if v is not None}
            if len(built)!=sum(v is not None for _,_,v in a):continue
            for b in itertools.product(*(move_go(u,1,built) for u,_,_ in a)):
                last={v for _,_,v in b if v is not None}
                if len(last)!=sum(v is not None for _,_,v in b) or last|built!=goal:continue
                if baseline_actions is None:return (0,0,0)
                cost=[0,0,0]
                for k,cmd in enumerate((a,b)):
                    for who,(p,c,g) in zip(pair,cmd):
                        orig=baseline_actions[k][who]
                        if orig!=c:
                            cost[0]+=1
                            cost[1]+=orig.islower()
                            cost[2]+=orig.isupper()
                # Do not let an untouched baseline human step into the future
                # pen or collide with a changed wall or previously built cap.
                ok=True
                for k in range(2):
                    closed=set() if not k else built
                    new=built if not k else last
                    for who in range(len(hs)):
                        if who in pair:continue
                        p=hs[who]
                        for i in range(k+1):
                            c=baseline_actions[i][who]
                            if c!='.' and c.isupper():
                                p+=dict(U=-30,D=30,L=-1,R=1)[c]
                        if p in closed|new or B[p]&interior:ok=False
                        c=baseline_actions[k][who]
                        if c.islower():
                            wall=p+dict(u=-30,d=30,l=-1,r=1)[c]
                            if wall in {v for v,_,_ in a+b} or wall in goal:ok=False
                if not ok:continue
                t=tuple(cost)
                if best is None or t<best:best=t
    return best

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ('harness','tester','solver','cases','states'):p.add_argument(n,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester,solver=a.tester.resolve(),a.solver.resolve()
    playback=a.harness.resolve()/'benchmark_cases.py'
    states=defaultdict(list)
    for x in csv.DictReader(a.states.open()):
        states[(x['case'],int(x['t']))].append(x)
    rooms=defaultdict(list)
    for (case,t),r in states.items():
        for x,y in itertools.combinations(r,2):
            if x['animal']==y['animal']:continue
            gs=[tuple(map(int,z['gates'].split('+'))) for z in (x,y)]
            distance=min(md(i,j) for i in gs[0] for j in gs[1])
            rooms[case].append((t,x,y,gs,distance))
    out=csv.writer(sys.stdout,lineterminator='\n')
    out.writerow(['case','turn','petA','petB','gatesA','gatesB','closest_mouth_cells_manhattan','joint_gate_count',
                  'independent_min_p4','interiors_disjoint','gates_outside_both','two_worker_two_turn_route',
                  'both_targets_inside_next_turn','legal_observed_and_preserve_unassigned','changed_commands',
                  'omitted_baseline_wall_jobs','omitted_baseline_move_jobs','merged_area','merged_gates','merged_worker_route','merged_observed_legal'])
    for case,tests in sorted(rooms.items()):
        payload=(a.cases/case).read_bytes()
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
        for t,x,y,(ga,gb),distance in tests:
            pets,humans,walls=hist[t]
            hs=tuple(map(idx,humans))
            opened=ALL^sum(B[idx(p)] for p in walls)
            ix,iy=int(x['animal']),int(y['animal'])
            left=flood(B[idx(pets[ix])],opened&~gatebs(ga))
            right=flood(B[idx(pets[iy])],opened&~gatebs(gb))
            assert left.bit_count()==int(x['area']) and right.bit_count()==int(y['area'])
            independent=not left&right
            gs=tuple(sorted(set(ga)|set(gb)))
            outboard=not gatebs(gs)&(left|right)
            body=left|right
            hcost=None; route=False
            now_inside=bool(left&B[idx(hist[t+1][0][ix])]) and bool(right&B[idx(hist[t+1][0][iy])])
            blocks=[]
            for k in (0,1):
                pp,hh,_=hist[t+k]
                part=0
                for p in pp:part|=B[idx(p)]
                hhbits=0
                for h in hh:hhbits|=B[idx(h)]
                blocks.append(part|wave(part)|hhbits)
            baseline=[data[2*t],data[2*(t+1)]]
            if distance<=2 and independent and outboard and len(gs)<=4:
                route=two_turn(opened,hs,gs,body,(0,0)) is not None
                if route and now_inside:
                    hcost=two_turn(opened,hs,gs,body,blocks,baseline)
            joint=body|gatebs(set(ga)&set(gb))
            area=joint.bit_count()
            merged_gates=tuple(cells(wave(joint)&opened&~joint))
            connected=flood(B[idx(pets[ix])],joint)==joint
            merged_valid=(distance<=2 and independent and outboard and connected and area<=24 and all(not B[h]&joint for h in hs))
            route_m=merged_valid and len(merged_gates)<=4 and two_turn(opened,hs,merged_gates,joint,(0,0)) is not None
            still=bool(joint&B[idx(hist[t+1][0][ix])]) and bool(joint&B[idx(hist[t+1][0][iy])])
            legal_m=route_m and still and two_turn(opened,hs,merged_gates,joint,blocks,baseline) is not None
            out.writerow([case,t,ix,iy,x['gates'],y['gates'],distance,len(gs),min(float(x['opt4']),float(y['opt4'])),
                          int(independent),int(outboard),int(route),int(now_inside),int(hcost is not None),*(hcost or (-1,)*3),area if merged_valid else -1,len(merged_gates) if merged_valid else -1,int(route_m),int(legal_m)])
if __name__=='__main__':main()
