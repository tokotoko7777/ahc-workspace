import csv,collections,hashlib,json,os,subprocess,sys,tempfile
from pathlib import Path
D=Path(__file__).resolve().parent;R=D.parent
sys.path.insert(0,str(R/'ahc008_terry_inward_gates'))
from screen import B,bitmask,flat,markers,near,observed,replay,wave
P=R/'ahc008_terry_oversized_chase';V=R/'ahc008_terry_virtual_chase_absorb'
def sha(bb):return hashlib.sha256(bb).hexdigest()
def dist(a,b,w):
    x=B[a];have=x;d=0
    while x:
        if x&B[b]:return d
        x=wave(x)&~(w|have);have|=x;d+=1
    return None
def inspect(exe,r,rec,folder,inp,maps):
    f=r['case'];raw=(inp/f).read_bytes()
    ap=(folder/f).with_suffix('.actions');a=ap.read_bytes();tr=ap.with_suffix('.trace').read_bytes()
    assert [sha(x)for x in (raw,a,tr)]==[rec[k]for k in ('input_sha','commands_sha','trace_sha')]
    ac=a.decode().splitlines();trace=tr.decode().splitlines()
    assert replay(raw.decode(),trace,ac,detailed=True)[0]==int(r['score'])
    _,hi,_=observed(raw,trace)
    lines=raw.decode().splitlines();n=int(lines[0]);m=int(lines[n+1])
    first={}
    for t,(_,h,w) in enumerate(hi[:-1]):
        for j,c in enumerate(ac[t]):
            if c.islower():
                p=near(flat(h[j]),c.upper())
                if B[p]&maps[int(r['map'])][0] and p not in first:first[p]=(t,j)
    plan=maps[int(r['map'])][0]
    frontier=max((t for t,j in first.values()),default=-1) if len(first)==plan.bit_count() else -1
    with tempfile.TemporaryDirectory()as d:
        fp=Path(d)/'log'
        z=subprocess.run([exe,'--pairs=0'],input='\n'.join(lines[:n+2+m]+trace[1:550:2])+'\n',
             capture_output=True,text=True,check=True,timeout=3,env=dict(os.environ,AHC008_FIFO_DEAD_LOG=str(fp)))
        assert z.stdout.splitlines()==ac[:275],f
        events=fp.read_text().splitlines()
    met=collections.Counter();seen=set();witness=[]
    for line in events:
        t,i,head,mask0,c,next_,bits=line.split()
        t,i,head,mask0,next_,bits=map(int,(t,i,head,mask0,next_,bits))
        if ac[t][i]!=c:continue
        met['actual_dead_walk_frames']+=1
        _,h,w=hi[t];p=flat(h[i]);walls=bitmask(map(flat,w))
        assert p!=head and all(walls&B[near(head,x)] for k,x in enumerate('URDL') if mask0>>k&1)
        key=(i,head,mask0)
        if key in seen:continue
        seen.add(key);met['distinct_episodes']+=1
        if next_<0:met['tail_episodes']+=1;continue
        d0=dist(p,head,walls);d1=dist(head,next_,walls);d2=dist(p,next_,walls)
        if None in (d0,d1,d2):met['disconnected']+=1;continue
        saving=d0+d1-d2;assert saving>=0
        met['nominal_detour_steps']+=saving
        met['episodes_at_least3']+=saving>=3
        goals={near(next_,x) for k,x in enumerate('URDL') if bits>>k&1}
        assert all(q is not None and plan&B[q] and not walls&B[q] for q in goals)
        work=[first[q] for q in goals if q in first and first[q][1]==i]
        critical=any(when==frontier and frontier>=0 for when,_ in work)
        met['productive_at_least3']+=bool(work) and saving>=3
        met['critical_at_least3']+=critical and saving>=3
        witness.append(dict(t=t,worker=i,head=head,next=next_,saving=saving,productive=bool(work),critical=critical))
    return dict(case=f,stats=met,witness=witness)
def audit(exe):
    out={};maps=markers()
    for panel,inp,dirs in [('practice',R/'ahc008_focus_assets/tools/in',[P/'practice-map1',P/'practice-late']),
      ('fresh',R/'ahc008_terry_first_chase_reuse/default-fresh100/in',[P/'fresh-map1',P/'fresh-late',R/'ahc008_terry_fresh_residual/tapes'])]:
        led={q['case']:q for q in csv.DictReader((V/f'{panel}.native.csv').open())if q['arm']=='terry_independent'}
        selected={}
        for d in dirs:
            rf=d/'recordings.json';rf=rf if rf.is_file() else d.parent/rf.name
            for q in json.loads(rf.read_text()):
                row=led[q['case']]
                assert all(q[k]==row[k]for k in ('input_sha','commands_sha'))
                selected[q['case']]=(q,d)
        assert len(selected)==(26 if panel=='practice' else 34)
        arr=[inspect(exe,led[case],q,d,inp,maps)for case,(q,d) in sorted(selected.items())]
        total=collections.Counter()
        for v in arr:total.update(v['stats'])
        out[panel]=dict(n=len(arr),total=total,boards=collections.Counter(k for v in arr for k,c in v['stats'].items()if c),cases=arr)
    print(json.dumps(out,indent=2))
if __name__=='__main__':audit(str(Path(sys.argv[1]).resolve()))
