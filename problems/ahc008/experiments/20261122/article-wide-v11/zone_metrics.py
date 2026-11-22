"""Observed occupancy and actual complete exact-four seals by bank/longitudinal third."""
from collections import defaultdict
from bridge import parse_geometry
from stochastic_lock_audit import idx
from parallel_banks_anchored_cut_audit import buf

def measure(hist,kinds,geo):
    _,_,tubes=parse_geometry(geo)
    states=[(list(map(idx,p)),buf(map(idx,w))) for p,h,w in hist]
    bins={}
    for x in tubes:
        zone=(x.bank,x.row//10)
        z=bins.setdefault(zone,dict(tubes=0,initial_ids=set(),open_turns=0,open_ready_turns=0,ready_ids=set(),
                                    ready_visits=[0]*5,pre_ready_visits=[0]*5,captured_ids=set(),sealed_tubes=0))
        z['tubes']+=1
        for i,p in enumerate(states[0][0]):
            if p in x.inside:z['initial_ids'].add(i)
        for t,((pets,w),(_,nextw)) in enumerate(zip(states,states[1:])):
            if w&(x.gates|x.body):continue
            ready=w&x.wall==x.wall
            z['open_turns']+=1
            z['open_ready_turns']+=ready
            seen={i for i,p in enumerate(pets) if p in x.inside}
            for i in seen:
                z['ready_visits' if ready else 'pre_ready_visits'][kinds[i]-1]+=1
            if ready:
                z['ready_ids'].update(seen)
                if nextw&x.gates==x.gates and not nextw&x.body:
                    z['sealed_tubes']+=1
                    z['captured_ids'].update(seen)
    for z in bins.values():
        for key in ('initial_ids','ready_ids','captured_ids'):
            z[key]=[sum(kinds[i]==k for i in z[key]) for k in range(1,6)]
    return {f'{a}:{b}':z for (a,b),z in bins.items()}
