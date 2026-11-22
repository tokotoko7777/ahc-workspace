"""Audit untargeted secondary crew, no changes to official narrow output."""
import argparse,csv,hashlib,json,sys,tempfile
from collections import Counter
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ADJ,ALL,B,idx,flood,wave
from parallel_banks_anchored_cut_audit import bits,buf,rect

def examine(log,hist,cmds,missed,name):
    data=log.read_text().splitlines()
    stride,release=map(int,data[0].split());assert stride==30
    rows=[]
    for line in data[1:]:
        t,mask,n,*seq=map(int,line.split())
        ww,body=buf(seq[:n]),buf(seq[n:])
        assert release<=t<300
        pp,hh,w=hist[t];origin=list(map(idx,hh))
        finish=list(map(idx,hist[t+1][1]))
        now=buf(map(idx,w));base=buf(map(idx,hist[t+1][2]))
        pps=list(map(idx,pp))
        pets=buf(pps);humans=buf(finish)
        blocked=pets|wave(pets)|humans|buf(origin)
        pending=ww&~base
        options=[(i,list(bits(wave(B[h])&pending&~blocked))) for i,h in enumerate(origin)
                 if mask>>i&1 and cmds[t][i]=='.' and not B[h]&(ww|body)]
        options=[(i,l) for i,l in options if l]
        rows.append(dict(case=name,turn=t,status='no_stance' if not options else 'unsafe',
                         idle=sum(mask>>i&1 and c=='.' for i,c in enumerate(cmds[t])),
                         pending=pending.bit_count()))
        if not options:continue
        open_=ALL^base
        parts=[]
        unseen=humans
        while unseen:
            g=flood(unseen&-unseen,open_);parts.append(g&humans);unseen&=~g
        old=flood(humans,open_)
        # Exposure is observed at this turn; account for a planned animal
        # only when the fully realized original wall footprint actually
        # separates it and does not split any current ordinary human group.
        wp=open_&~pending
        def unsplit(wp):return all(flood(g&-g,wp)&g==g for g in parts)
        exposure=old&~flood(humans,wp)&pets if unsplit(wp) else 0
        def ids(mask):return [i for i,q in enumerate(pps) if B[q]&mask]
        best=None
        def search(k,new,assign):
            nonlocal best
            if k==len(options):
                if not new:return
                opened=open_&~new
                if not unsplit(opened):return
                detach=old&~flood(humans,opened)&pets
                ii=ids(detach)
                score=(sum(j in missed for j in ii),len(ii),new.bit_count())
                if best is None or score>best[0]:
                    geometry=[]
                    for j in ii:
                        pen=flood(B[pps[j]],opened)
                        geometry.append(dict(pet=j,area=pen.bit_count(),nonrect=not rect(pen)))
                    best=(score,dict(status='safe',added=list(assign),capture=geometry,
                                     missed_capture=[j for j in ii if j in missed],
                                     exposure=ids(exposure),
                                     missed_exposure=[j for j in ids(exposure) if j in missed]))
                return
            i,choice=options[k]
            search(k+1,new,assign)
            for a in choice:
                if not new&B[a]:search(k+1,new|B[a],assign+[(i,a)])
        search(0,0,[])
        if best:rows[-1].update(best[1])
    return rows

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','tester','cases','solver','control_csv','output','summary'):
        p.add_argument(k,type=Path)
    a=p.parse_args()
    sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with a.control_csv.open() as f: refs=list(csv.DictReader(f))
    rows=[]
    for ref in refs:
        name=ref['case'];case=(a.cases/name).read_bytes()
        with tempfile.TemporaryDirectory() as td:
            d=Path(td);log=d/'log';act=d/'act';tr=d/'trace';test=a.tester.resolve()
            o,e,rc,_=run([test,'env',f'AHC008_BORROW_LOG={log}',a.solver.resolve()],case,3.)
            assert not rc and e.strip()==('Score = '+ref['candidate']).encode()
            assert hashlib.sha256(o).hexdigest()==ref['candidate_actions_sha256']
            act.write_bytes(o)
            pb,e2,rc2,_=run([test,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                             '--playback',act,tr],case,10.)
            assert (pb,e2,rc2)==(o,e,0)
            lines=tr.read_text().splitlines();cmds=o.decode().splitlines()
            assert replay(case.decode(),lines,cmds,detailed=True)[0]==int(ref['candidate'])
            _,hist,_=observed(case,lines)
            pp,hh,w=hist[-1]
            keep=flood(buf(map(idx,hh)),ALL^buf(map(idx,w)))
            miss={i for i,q in enumerate(map(idx,pp)) if B[q]&keep}
            rows.extend(examine(log,hist,cmds,miss,name))
        print(name,flush=True)
    safe=[r for r in rows if r['status']=='safe']
    def ids(k):return len({(r['case'],j) for r in safe for j in r[k]})
    s=dict(verified=len(refs),candidate_turns=len(rows),case_count=len({r['case'] for r in rows}),
           status=dict(Counter(r['status'] for r in rows)),safe_cases=len({r['case'] for r in safe}),
           safe_builds=sum(len(r['added']) for r in safe),capture_turns=sum(bool(r['capture']) for r in safe),
           capture_pets=len({(r['case'],p['pet']) for r in safe for p in r['capture']}),
           final_missed_capture=ids('missed_capture'),final_missed_exposure=ids('missed_exposure'),
           final_missed_exposure_cases=len({r['case'] for r in safe if r['missed_exposure']}))
    a.output.write_text(''.join(json.dumps(r,separators=(',',':'))+'\n' for r in rows))
    a.summary.write_text(json.dumps(s,indent=2)+'\n');print(json.dumps(s,indent=2))
if __name__=='__main__':main()
