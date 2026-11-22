"""Matched exact-four-cell pilot with official tester, action replay and telemetry."""
import argparse,csv,hashlib,re,sys,tempfile
from pathlib import Path
from collections import Counter
from guarded_entry_trace import observed
from stochastic_lock_audit import ADJ,ALL,B,idx,flood,wave
ARMS=(('control',0,0),('fixed-square',2,0),('deferred-square',2,1))
def all_pen_counts(pets,humans,walls,kinds):
    opened=ALL^sum(B[idx(w)] for w in walls)
    reachable=flood(sum({B[idx(x)] for x in humans}),opened)
    out=Counter()
    for p,k in zip(pets,kinds):
        bit=B[idx(p)]
        if not reachable&bit and flood(bit,opened).bit_count()==4:out[k]+=1
    return out,len([p for p in pets if reachable&B[idx(p)]])

def stats(hist,kinds,side):
    m={}
    final,res=all_pen_counts(*hist[-1],kinds)
    for k in range(1,6):m['final4_'+str(k)]=final[k]
    m['final_reachable']=res
    if not side:return m
    items=side.splitlines()
    _,kind,g,estimate,guard,b0,b1,*left=items[0].split()
    if int(g)<0:return m|{'planned':0}
    a=list(map(int,left))
    body=set(a[:4]);gate,stance=a[4:6]
    shell=set(a[6:])
    shoulders=shell-{gate}
    guard=int(guard);builders={int(b0),int(b1)}
    bits=sum(B[x] for x in body)
    assert flood(B[a[0]],bits)==bits
    assert (wave(bits)&~bits)==sum(B[x] for x in shell)
    m.update(planned=1,planned_ready=int(estimate),shoulders=len(shoulders),ready_turn=-1,staffed_turns=0,
             guard_setup_walk=0,guard_watch_commands=0,build_cmds=0,builder_walk=0,builder_wait=0,
             diverted_commands=0,diverted_original_walls=0,diverted_original_moves=0,
             shell_built=0,seal=-1,legal_watch_turns=0,arrivals=0,already_in_at_ready=0,
             full_preparation_zero=0,full_legal_ready=0,full_legal_two_or_more_gates=0,
             final_jump=0,first_build_turn=-1,active_crew_moves=0,early_crew_waits=0,
             actual_last_walls='')
    seen=set();newly=set();captures=set();legal=set()
    for row in items[1:]:
        s,prior,actions,ready,done,end,mask=row.split()
        t=int(s);owned=int(mask);end=int(end)
        if end>=0 and t>=end:break
        pp,hh,ww=hist[t]; pn,hn,wn=hist[t+1]
        ps=list(map(idx,pp));prev=list(map(idx,hist[max(0,t-1)][0]))
        h=list(map(idx,hh)); w=set(map(idx,ww)); new=set(map(idx,wn))-w
        m['shell_built']+=len(new&shell)
        pending=shell-w
        if new&shell and m['first_build_turn']<0:m['first_build_turn']=t
        need=list(sorted(pending))
        occupied=set(h);ppos=set(ps)
        blocked=occupied|ppos|{q for p in ps for q in ADJ[p]}
        def cover(j,used,with_block):
            if j==len(need):return True
            g=need[j]
            if with_block and g in blocked:return False
            return any(cover(j+1,used|{i},with_block) for i,u in enumerate(h)
                       if i not in used and u not in shell|body and g in ADJ[u])
        targets={i for i,p in enumerate(ps) if p in body}
        if targets and pending and not occupied&body:
            m['full_preparation_zero']+=cover(0,set(),False)
            if cover(0,set(),True):
                m['full_legal_ready']+=1
                m['full_legal_two_or_more_gates']+=len(pending)>=2
        for i,(before,after) in enumerate(zip(prior,actions)):
            if before!=after:
                m['diverted_commands']+=1
                m['diverted_original_walls']+=before.islower()
                m['diverted_original_moves']+=before.isupper()
            if not (owned>>i&1):continue
            m['active_crew_moves']+=after.isupper()
            m['early_crew_waits']+=len(pending)>1 and after=='.'
            if i in builders:
                m['build_cmds']+=after.islower()
                m['builder_walk']+=after.isupper()
                m['builder_wait']+=after=='.'
            if i==guard:
                m['guard_setup_walk']+=after.isupper()
                m['guard_watch_commands']+=after=='.'
        ready_now=len(pending)==1
        active=ready_now and not w&body
        if active and m['ready_turn']<0:m['ready_turn']=t
        targets={i for i,p in enumerate(ps) if p in body}
        if active:
            seen.update(targets)
            for i in targets:
                if prev[i] not in body:newly.add(i)
            if m['ready_turn']==t:m['already_in_at_ready']=len(targets)
        if active and h[guard]==stance and gate in pending:
            m['staffed_turns']+=1
            forbidden=set(ps)|set(h)
            forbidden|={q for p in ps for q in ADJ[p]}
            if gate not in forbidden and targets:m['legal_watch_turns']+=1;legal.update(targets)
        if pending and pending<=new and not w&body and not (new&body):
            opened=ALL^sum(B[idx(x)] for x in wn)
            assert not (set(map(idx,hn))&body)
            for i in targets:
                assert idx(pn[i]) in body and flood(B[idx(pn[i])],opened)==bits
            captures.update(targets)
            m['seal']=t
            m['final_jump']=len(pending)
            m['actual_last_walls']='+'.join(map(str,sorted(pending)))
    m['arrivals']=len(seen);m['new_after_ready']=len(newly);m['capture4']=len(captures);m['legal_target_count']=len(legal)
    for k in range(1,6):
        m['arrivals_'+str(k)]=sum(kinds[i]==k for i in seen)
        m['new_arrivals_'+str(k)]=sum(kinds[i]==k for i in newly)
        m['capture_'+str(k)]=sum(kinds[i]==k for i in captures)
    return m

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for n in ['harness','tester','baseline','pilot','cases','out']:p.add_argument(n,type=Path)
    p.add_argument('--case-count',type=int,default=30)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    tester=a.tester.resolve();playback=a.harness.resolve()/'benchmark_cases.py'
    rows=[]
    files=sorted(a.cases.glob('*.txt'))[:a.case_count]
    for f in files:
        case=f.read_bytes()
        config=None
        for tag,k,deferral in ARMS:
            with tempfile.TemporaryDirectory() as td:
                probe=Path(td)/'quad'
                command=[tester,a.baseline.resolve()] if k==0 else [tester,'env',f'AHC008_QUAD_LOG={probe}',a.pilot.resolve(),str(k),'27',str(deferral)]
                stdout,stderr,code,time=run(command,case,3.)
                match=re.fullmatch(rb'Score = (\d+)\s*',stderr)
                assert code==0 and match,(f.name,tag,stderr)
                act=Path(td)/'actions';act.write_bytes(stdout);trace=Path(td)/'trace'
                stdout2,stderr2,c2,_=run([tester,sys.executable,playback,'--playback',act,trace],case,10.)
                assert (stdout2,stderr2,c2)==(stdout,stderr,0)
                data=trace.read_text().splitlines()
                score,*_=replay(case.decode(),data,stdout.decode().splitlines(),detailed=True)
                assert int(match[1])==score
                kinds,hist,_=observed(case,data)
                row={'case':f.name,'model':tag,'score':score,'wall_s':round(time,6),
                     'input_sha256':hashlib.sha256(case).hexdigest(),'actions_sha256':hashlib.sha256(stdout).hexdigest()}
                side=probe.read_text() if k else None
                if k:
                    current=side.splitlines()[0]
                    if config is None:config=current
                    assert config==current,(f.name,tag)
                row.update(stats(hist,kinds,side))
                rows.append(row)
        fields=list(dict.fromkeys(k for row in rows for k in row))
        with a.out.open('w') as file:
            writer=csv.DictWriter(file,fields,lineterminator='\n');writer.writeheader();writer.writerows(rows)
        print(f.name,flush=True)
if __name__=='__main__':main()
