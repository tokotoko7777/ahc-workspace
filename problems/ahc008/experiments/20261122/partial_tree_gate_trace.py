"""Count actual isolated animals attributable to proposed partial-tree gates."""
import argparse,csv,re,sys,tempfile
from pathlib import Path
from collective_cut_audit import BOARD,N
from collective_baffle_audit import flood
from guarded_entry_trace import observed

SETUP=(3,7)
CLOSE=(7,3)

def inspect(history):
    setup=seal=-1
    incremental=set(); eligible=guard=0
    for t in range(40,123):
        pets,oldhumans,old=history[t]
        if SETUP in old and CLOSE not in old:
            leaf=flood([(3,3)],BOARD-(old|{CLOSE}))
            if leaf and all(r<7 and c<7 for r,c in leaf) and any(p in leaf for p in pets) and not leaf&set(oldhumans):
                if CLOSE not in pets and CLOSE not in oldhumans and not set(N[CLOSE])&set(pets):
                    eligible+=1
                    guard+=bool((set(N[CLOSE])-leaf)&set(oldhumans))
        _,humans,new=history[t+1]
        if SETUP in new-old:setup=t
        if CLOSE in new-old:
            seal=t
            after=flood(humans,BOARD-new)
            without=flood(humans,BOARD-(new-{CLOSE}))
            incremental.update(i for i,p in enumerate(pets) if p in without-after)
    return setup,seal,len(incremental),eligible,guard

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for key in ("harness","tester","base","tree","cases"):p.add_argument(key,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester=a.tester.resolve()
    script=a.harness.resolve()/"benchmark_cases.py"
    out=csv.writer(sys.stdout,lineterminator="\n")
    out.writerow(["case","solver","setup","target_portal_close","isolated_pets_attributable_to_target_portal","postsetup_legal_pure_leaf_turns","postsetup_already_staffed_turns"])
    for f in sorted(a.cases.glob("*.txt")):
        case=f.read_bytes()
        for name,path in [("control",a.base),("partial",a.tree)]:
            cmd,err,rc,_=run([tester,path.resolve()],case,3.)
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
            out.writerow([f.name,name,*inspect(h)])
            sys.stdout.flush()
if __name__=="__main__":main()
