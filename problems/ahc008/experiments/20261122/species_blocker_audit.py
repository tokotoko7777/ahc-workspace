"""Necessary animal removals for macro clean unions on observed legal traces."""
import argparse,csv,re,sys,tempfile
from pathlib import Path
from collective_cut_audit import layout,parts,N,TIMES
from guarded_entry_trace import observed

def candidate_rooms():
    region,portals,adj,cells=layout(4,4)
    # Same large union model as collective cut audit, but enumerate every
    # rectangle and every connected one-turn bend (orthogonally convex).
    candidates=[]
    for mask in range(1,1<<16):
        v={i for i in range(16) if mask>>i&1}
        if sum(len(cells[i]) for i in v)<280:continue
        if len(next(parts(v,adj)))!=len(v):continue
        if any(s and max(s)-min(s)+1!=len(s) for vline in range(8)
               for s in [[i%4 if vline<4 else i//4 for i in v if (i//4 if vline<4 else i%4+4)==vline]]):continue
        domain=set().union(*(cells[i] for i in v))
        domain.update(p for p,neighbors in portals.items() if neighbors<=v)
        if len(domain)>=300:candidates.append(domain)
    return candidates

ALL=(1<<900)-1
LEFT=sum(1<<(i*30) for i in range(30))
RIGHT=LEFT<<29

def bit(p):
    return 1<<(p[0]*30+p[1])

def components(live):
    while live.bit_count()>=300:
        reach=live&-live
        while True:
            x=reach|((reach&~RIGHT)<<1)|((reach&~LEFT)>>1)|(reach<<30)|(reach>>30)
            x&=live
            if x==reach:break
            reach=x
        live^=reach
        if reach.bit_count()>=300:yield reach

def measure(domains,hist,types):
    species=[1,2,3,4,5]
    counts={threshold:[len(types)+1]*7 for threshold in (300,450)}
    witnesses={}
    for t in TIMES:
        ps,hs,walls=hist[t]
        passable=ALL^sum(bit(w) for w in walls)
        pbits=list(map(bit,ps))
        for d in domains:
            for comp in components(d&passable):
                area=comp.bit_count()
                own=[i for i,p in enumerate(pbits) if p&comp]
                for threshold in counts:
                    if area<threshold:continue
                    tally=counts[threshold]
                    if (len(own),-area,t)<witnesses.get(threshold,((len(types)+1,0,0),None))[0]:
                        witnesses[threshold]=((len(own),-area,t),tuple(sum(types[i]==s for i in own) for s in species),comp,own)
                    tally[0]=min(tally[0],len(own))
                    for s in species:
                        tally[s]=min(tally[s],sum(types[i]!=s for i in own))
                    tally[6]=min(tally[6],sum(types[i] not in (4,5) for i in own))
    for th in counts:
        w=witnesses.get(th)
        if not w:
            yield [th,*counts[th],-1,0,*([0]*12)]
            continue
        info,by_species,region,own=w
        t=info[2]
        past=[0,0]
        future=[0,0]
        inbound=[0,0]
        own_set=set(own)
        # Geometric witness held fixed while evaluating observed passage.
        for j,H in enumerate((10,20)):
            past[j]=sum(any(not (region&bit(hist[k][0][i])) for k in range(t-H,t)) for i in own)
            future[j]=sum(any(not (region&bit(hist[k][0][i])) for k in range(t+1,t+H+1)) for i in own)
            inbound[j]=sum(any(region&bit(hist[k][0][i]) for k in range(t+1,t+H+1)) for i in range(len(types)) if i not in own_set)
        later_clear=any(not any(region&bit(p) for p in hist[k][0]) for k in range(t+1,t+21))
        yield [th,*counts[th],t,-info[1],*by_species,*past,*future,*inbound,int(later_clear)]

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ("harness","tester","solver","cases"):p.add_argument(key,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester,solver=a.tester.resolve(),a.solver.resolve()
    script=a.harness.resolve()/"benchmark_cases.py"
    domains=[sum(map(bit,d)) for d in candidate_rooms()]
    print(f"{len(domains)} convex macro room unions",file=sys.stderr)
    out=csv.writer(sys.stdout,lineterminator="\n")
    out.writerow(["case","threshold","blockers","remove_cows","remove_pigs","remove_rabbits","remove_dogs","remove_cats","remove_dogs_cats","witness_turn","witness_area","cows","pigs","rabbits","dogs","cats","witness_blockers_outside_past10","witness_blockers_outside_past20","witness_blockers_exit_next10","witness_blockers_exit_next20","initial_outsiders_enter_next10","initial_outsiders_enter_next20","witness_ever_empty_next20"])
    for f in sorted(a.cases.glob("*.txt")):
        case=f.read_bytes()
        lines=case.decode().splitlines()
        types=[int(row.split()[2]) for row in lines[1:int(lines[0])+1]]
        cmd,err,rc,_=run([tester,solver],case,3.)
        match=re.fullmatch(rb"Score = (\d+)\s*",err)
        assert rc==0 and match
        with tempfile.TemporaryDirectory() as td:
            actions=Path(td)/"actions";actions.write_bytes(cmd)
            trace=Path(td)/"trace"
            actual,e,c,_=run([tester,sys.executable,script,"--playback",actions,trace],case,10.)
            assert c==0 and actual==cmd and e==err
            data=trace.read_text().splitlines()
        s,*_=replay(case.decode(),data,cmd.decode().splitlines(),detailed=True)
        assert s==int(match[1])
        _,h,_=observed(case,data)
        for row in measure(domains,h,types):out.writerow([f.name,*row])
        sys.stdout.flush()
if __name__=="__main__":main()
