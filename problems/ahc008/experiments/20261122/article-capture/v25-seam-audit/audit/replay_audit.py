"""Summarize one already recorded official tape plus solver lifecycle log; never reruns."""
from pathlib import Path
from collections import defaultdict
import argparse,json,sys
W=Path(__file__).resolve().parent.parent
H=W/'ahc008_focus_solver/problems/ahc008'
sys.path[:0]=[str(H),str(H/'experiments/20261122')]
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,idx,flood
from parallel_banks_anchored_cut_audit import buf
from benchmark import replay
def inspect(raw,actions,pets,trace):
    # Existing independent validator verifies all recorded commands.
    score,*_=replay(raw.decode(),pets,actions,detailed=True)
    kinds,hist,_=observed(raw,pets)
    assert len(hist)==301 and len(actions)==300
    st=[]
    for ps,hs,ws in hist:
        h,p=tuple(map(idx,hs)),tuple(map(idx,ps))
        w=buf(map(idx,ws))
        ext=flood(buf(h),ALL^w)
        areas=[flood(B[q],ALL^w).bit_count() for q in h]
        st.append((w,ext,areas,{i for i,q in enumerate(p) if ext&B[q]}))
    episodes={}
    for line in trace:
        z=line.split()
        t,stage,sid,start,root,root_area,root_pets,floor,projected,expected_pets,eta,walls,mode=map(int,z[:13])
        raw,repaired,gated,sent=z[13:17]
        assert sent==actions[t],(t,sent,actions[t])
        cells=set(map(int,z[17:]))
        if sid<0 or start<178:continue
        key=(start,sid)
        if stage==2 and key not in episodes:
            episodes[key]=dict(start=start,line=sid,eta=eta,planned=sorted(cells),keep=projected,
                               projected_pets=expected_pets,required=floor,root_area=root_area,turns=[])
        if key not in episodes:continue
        ep=episodes[key]
        once=buf(ep['planned'])
        before,after=st[t:t+2]
        changed=after[0]&~before[0]
        newheads=before[3]-after[3]
        finalwall=bool(once) and once&after[0]==once and once&before[0]!=once
        ep['turns'].append(dict(t=t,stage=stage,mode=mode,root=root,rootArea=root_area,
            actual_union=[before[1].bit_count(),after[1].bit_count()],actual_per_human=[before[2],after[2]],
            raw=raw,repaired=repaired,gated=gated,sent=sent,
            submitted_line_walls=(changed&once).bit_count(),other_walls=(changed&~once).bit_count(),
            completed_physical=finalwall,captured=[dict(pet=i,kind=kinds[i]) for i in sorted(newheads)],
            retainedAfter=projected,retainedPets=expected_pets,
            repaired_override=raw!='-' and raw!=repaired,gate_override=gated!='-' and gated!=repaired,
            terminal_override=gated!='-' and gated!=sent))
    for ep in episodes.values():
        q=ep['turns'];closed=[e for e in q if e['completed_physical']]
        ep['actual_completion_turn']=closed[0]['t'] if closed else None
        # Factual captures on a mixed command's turn are not solely attributed
        # to the seam unless there were no other simultaneous wall commands.
        ep['closure_heads']=closed[0]['captured'] if closed else []
        ep['closure_has_unrelated_walls']=bool(closed and closed[0]['other_walls'])
        ep['stall_turns']=sum(e['stage']==2 and set(e['sent'])=={'.'} for e in q)
        ep['retired_without_completion']=not bool(closed)
    return dict(score=score,early_episodes=list(episodes.values()))
if __name__=='__main__':
 p=argparse.ArgumentParser()
 for k in ('input','actions','pets','trace'):p.add_argument(k,type=Path)
 a=p.parse_args()
 print(json.dumps(inspect(a.input.read_bytes(),a.actions.read_text().splitlines(),
  a.pets.read_text().splitlines(),a.trace.read_text().splitlines()),indent=2))
