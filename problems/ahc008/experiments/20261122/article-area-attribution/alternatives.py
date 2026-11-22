"""Exact same-turn alternative walls on 16 observed broad v4 late cuts."""
import argparse,csv,gzip,hashlib,json,sys,tempfile
from pathlib import Path
from guarded_entry_trace import observed
from stochastic_lock_audit import ALL,B,ADJ,idx,wave,flood
from parallel_banks_anchored_cut_audit import bits,buf
from audit import snapshot

def search(ev,hist):
    t=ev['turn'];pp,hh,w=hist[t];_,h2,w2=hist[t+1]
    p=list(map(idx,pp));h=list(map(idx,hh));after=list(map(idx,h2))
    old=buf(map(idx,w));new=buf(map(idx,w2));locked=new&~old
    assert locked.bit_count()==1 and ev['sources']=={'late':1}
    hbits=buf(h);pbits=buf(p)
    o=ALL^old;forbidden=hbits|pbits|wave(pbits)|old|locked
    moves=sum((h[i]!=after[i])<<i for i in range(len(h)))
    prior,_,pair0=snapshot(o,after,p)
    base,base_area,pair_base=snapshot(o&~locked,after,p)
    assert pair0==pair_base
    target=[i for i,x in enumerate(p) if (B[x]&prior) and not (B[x]&base)]
    assert sorted(target)==sorted(j for x in ev['detached'] for j in x['animals'])
    opts=[list(bits(wave(B[j])&~forbidden)) for j in h]
    seen=set()
    count=[0,0];keep_count=[0,0];best=[None,None]
    def examine(w,d,assign):
        k=(w,d)
        if k in seen:return
        seen.add(k)
        no_move=int(bool(d))
        count[no_move]+=1
        ff=[h[i] if (d>>i&1) else after[i] for i in range(len(h))]
        if w&buf(ff):return
        op=o&~w
        actual,area,npair=snapshot(op,ff,p)
        if npair!=pair0 or any(B[p[j]]&actual for j in target):return
        keep_count[no_move]+=1
        gain=(area-base_area)/len(h)
        caught=[i for i,x in enumerate(p) if B[x]&prior and not (B[x]&actual)]
        q=dict(gain=gain,caught=caught,added=list(assign),moves_changed=d.bit_count())
        compare=(gain,len(caught),-len(assign),-d.bit_count())
        if best[no_move] is None or compare>best[no_move][0]:best[no_move]=(compare,q)
    def walk(st,w,d,assign):
        if assign:examine(w,d,assign)
        if len(assign)>=3:return
        for i in range(st,len(h)):
            for q in opts[i]:
                if not w&B[q]:walk(i+1,w|B[q],d|(moves&(1<<i)),assign+[(i,q)])
    walk(0,0,0,[])
    return dict(case=ev['case'],turn=t,original_floor_loss=ev['human_floor_loss'],target=target,
                enumeration=count,same_yield=keep_count,best=[x[1] if x is not None else None for x in best])

def main():
    p=argparse.ArgumentParser(description=__doc__)
    for k in ('harness','tester','solver','cases','reference','prior','output'):
        p.add_argument(k,type=Path)
    a=p.parse_args();sys.path.insert(0,str(a.harness.resolve()))
    from benchmark_cases import run
    from benchmark import replay
    with open(a.reference) as f:refs={r['case']:r for r in csv.DictReader(f) if r['model']=='article'}
    op=gzip.open if a.prior.suffix=='.gz' else open
    with op(a.prior,'rt') as f:
        ev=[x for x in map(json.loads,f) if x['sources']=={'late':1} and x['human_floor_loss']>=100]
    assert len(ev)==16 and len({z['case'] for z in ev})==14
    output=[]
    for name in sorted({z['case'] for z in ev}):
        c=(a.cases/name).read_bytes();r=refs[name]
        with tempfile.TemporaryDirectory() as tmp:
            d=Path(tmp);act=d/'action';tr=d/'trace';tester=a.tester.resolve()
            o,e,rc,_=run([tester,a.solver.resolve()],c,3.)
            assert rc==0 and e.strip()==('Score = '+r['score']).encode()
            assert hashlib.sha256(o).hexdigest()==r['actions_sha256']
            act.write_bytes(o)
            po,pe,pr,_=run([tester,sys.executable,a.harness.resolve()/'benchmark_cases.py',
                            '--playback',act,tr],c,10.)
            assert (po,pe,pr)==(o,e,0)
            lines=tr.read_text().splitlines()
            assert replay(c.decode(),lines,o.decode().splitlines(),detailed=True)[0]==int(r['score'])
            _,hist,_=observed(c,lines)
            for e in ev:
                if e['case']==name:output.append(search(e,hist))
        print(name,flush=True)
    a.output.write_text(''.join(json.dumps(x,separators=(',',':'))+'\n' for x in output))
    print(json.dumps({"verified_cases":14,"audited_cuts":16,
          "strict_ge50":sum(x['best'][0] is not None and x['best'][0]['gain']>=50 for x in output),
          "replace_move_ge50":sum(x['best'][1] is not None and x['best'][1]['gain']>=50 for x in output),
          "same_yield_counts":[sum(x['same_yield'][i] for x in output) for i in range(2)],
          "enumeration":[sum(x['enumeration'][i] for x in output) for i in range(2)]},indent=2))
if __name__=='__main__':main()
