"""Counterfactual T-guard opportunity audit with explicit shared-stance shapes."""
import argparse,csv,re,sys,tempfile
from itertools import product
from pathlib import Path
from collective_cut_audit import layout,BOARD,N,short
from collective_baffle_audit import start_time
from guarded_entry_trace import observed

DIR=((0,1),(-1,0),(0,-1),(1,0))

def add(p,d,k=1):
    return p[0]+d[0]*k,p[1]+d[1]*k

def hubs():
    room,portals,_,_=layout(4,4)
    starts=(0,8,16,23)
    ends=(6,14,21,29)
    normal={(x,(a+b)//2) for x in (7,15,22) for a,b in zip(starts,ends)}
    normal|={((a+b)//2,y) for y in (7,15,22) for a,b in zip(starts,ends)}
    mandatory=BOARD-room.keys()-normal
    near={q for p in normal for q in N[p] if q in room}
    near|={q for p in list(near) for q in N[p] if q in room}
    choices=[]
    for q in sorted(near):
        slots=[]
        for d in range(4):
            gate=add(q,DIR[d]);first=add(q,DIR[d],2)
            arms=[]
            for d2,d3 in product((d,(d+1)%4,(d-1)%4),repeat=2):
                b=add(first,DIR[d2]);c=add(b,DIR[d3])
                inner={first,b,c}
                if len(inner)!=3 or not (inner|{gate})<=BOARD or (inner|{gate})&normal:
                    continue
                safe=[p for p in inner if abs(p[0]-gate[0])+abs(p[1]-gate[1])>=2]
                if {sum(p)%2 for p in safe}!={0,1}:continue
                outer={p for u in inner for p in N[u] if p not in inner and p!=gate}
                if q in outer or outer&normal or inner&mandatory:continue
                arms.append((gate,frozenset(inner),frozenset(outer)))
            slots.append(arms)
        for d in range(4):
            stem={add(q,DIR[d],k) for k in (1,2)}
            if not stem<=BOARD or stem&mandatory:continue
            for arms in product(*(slots[e] for e in range(4) if d!=e)):
                if any((inside|{gate}|boundary)&stem for gate,inside,boundary in arms):continue
                ok=True
                for i,(gate,inside,boundary) in enumerate(arms):
                    for j,(other,adj,adj_wall) in enumerate(arms):
                        if i!=j and ((inside|{gate})&(adj|{other}|adj_wall) or (adj|{other})&boundary):
                            ok=False
                if not ok:continue
                walls=set().union(*(z[2] for z in arms))
                cost=len(walls-mandatory)
                if cost<=15:
                    choices.append((cost,q,arms,walls&mandatory))
    if not choices:return [],0
    choices.sort(key=lambda x:(x[0],x[1],tuple(z[0] for z in x[2])))
    seen={}
    for ch in choices:
        key=(ch[1],tuple(z[0] for z in ch[2]))
        if key not in seen or len(seen[key])<2:seen.setdefault(key,[]).append(ch)
    return [c for a in seen.values() for c in a],len(choices)

def measure(hist,sample,actual_only=False):
    stop,_=start_time(hist)
    stop=220 if stop<0 else min(stop,220)
    maxlen=[0]*3; staffed=[0]*3; opportunities=0
    bad=[]
    for t in range(stop):
        pets,humans,wall=hist[t]
        bad.append(set(pets)|set(humans)|{u for p in pets for u in N[p]})
    for _,q,arms,support in sample:
        if actual_only:support=set().union(*(z[2] for z in arms))
        events= [[] for _ in range(stop)]
        for t in range(35,stop):
            pets,humans,wall=hist[t]
            if q in wall or support-wall:continue
            for k,(p,X,_) in enumerate(arms):
                if p in bad[t] or p in wall or X&wall or any(h in X for h in humans):continue
                mask=sum(1<<i for i,r in enumerate(pets) if r in X)
                if mask:
                    events[t].append((k,mask));opportunities+=1
        if not any(events):continue
        for i,L in enumerate((10,20,40)):
            for begin in range(35,max(35,stop-L+1),5):
                states={(0,0)}
                for t in range(begin,begin+L):
                    if not events[t]:continue
                    nxt=set(states)
                    for assigned,pets in states:
                        for arm,mask in events[t]:
                            if not assigned>>arm&1:nxt.add((assigned|1<<arm,pets|mask))
                    states=nxt
                val=max(m.bit_count() for _,m in states)
                maxlen[i]=max(maxlen[i],val)
                if val>staffed[i]:
                    _,hs,ws=hist[begin]
                    if any(q in short(h,2,ws) for h in hs):staffed[i]=val
    return [len(hist[0][0]),stop,opportunities,*maxlen,*staffed]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("harness",type=Path);p.add_argument("tester",type=Path);p.add_argument("solver",type=Path);p.add_argument("cases",type=Path)
    p.add_argument("--actual-only",action="store_true",help="require all arm walls already built")
    a=p.parse_args()
    sample,not_deduped=hubs()
    print(f"raw {not_deduped} distinct sampled {len(sample)}",file=sys.stderr)
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester,solver=a.tester.resolve(),a.solver.resolve()
    script=a.harness.resolve()/"benchmark_cases.py"
    out=csv.writer(sys.stdout,lineterminator="\n")
    out.writerow(["case","pets","stop","eligible_windows","oracle10","oracle20","oracle40","guard_near10","guard_near20","guard_near40"])
    for f in sorted(a.cases.glob("*.txt")):
        payload=f.read_bytes()
        cmd,err,rc,_=run([tester,solver],payload,3.)
        match=re.fullmatch(rb"Score = (\d+)\s*",err)
        assert rc==0 and match
        with tempfile.TemporaryDirectory() as td:
            path=Path(td)/"actions";path.write_bytes(cmd)
            trace=Path(td)/"trace"
            actual,e,c,_=run([tester,sys.executable,script,"--playback",path,trace],payload,10.)
            assert c==0 and actual==cmd and e==err
            lines=trace.read_text().splitlines()
        s,*_=replay(payload.decode(),lines,cmd.decode().splitlines(),detailed=True)
        assert s==int(match[1])
        _,h,_=observed(payload,lines)
        out.writerow([f.name,*measure(h,sample,a.actual_only)])
        sys.stdout.flush()
if __name__=="__main__":main()
