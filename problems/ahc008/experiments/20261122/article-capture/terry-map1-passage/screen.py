"""Fixed actual pre-capture board, exact finite-time passive first passage."""
import collections, csv, hashlib, json, statistics, sys
from pathlib import Path
import numpy as np
D=Path(__file__).resolve().parent;R=D.parent
sys.path.insert(0,str(R/'ahc008_terry_inward_gates'))
from screen import ALL,B,bitmask,cells,flat,flood,markers,near,observed,replay,wave
CAT=R/'ahc008_terry_small_rooms/catalog.tsv'; P=R/'ahc008_terry_oversized_chase'
V=900; H=(10,25,50)
def sha(x):return hashlib.sha256(x).hexdigest()
ADJ=[tuple(cells(wave(b))) for b in B]
def phy(w,g,seed):return flood(B[seed],ALL^(w|g))
def layout():
    X,_=markers()[1]; wide=[];narrow=[]
    for row in CAT.read_text().splitlines():
        m,i,l,r,*body=map(int,row.split())
        if m!=1:continue
        g=bitmask(x for x in (l,r) if x>=0)
        bn=bitmask(body)
        # Establish intact physical footprint independently on exact
        # pristine completed map, with only own configured gates shut.
        b=phy(X,g,min(body))
        if bn&~b or not g or b&g:continue
        if len(body)==9 and b==bn and g.bit_count()==2:wide.append((i,g,b))
        if 1<=b.bit_count()<=4:narrow.append((i,g,b))
    assert len(wide)==14
    return wide,narrow
def targets(W,hum,narrow):
    H=bitmask(hum); exterior=[flood(B[p],ALL^W) for p in hum]
    asc=0;n=0
    for _,g,b in narrow:
        if (g|b)&(W|H):continue
        if not any(ex&b for ex in exterior):continue
        seed=(b&-b).bit_length()-1
        if phy(W,g,seed)!=b:continue
        if g.bit_count()==2 and any(phy(W,B[s],seed)==b for s in cells(g)):continue
        if any(old&~(g|b)&~flood(B[p],ALL^(W|g)) for p,old in zip(hum,exterior,strict=True)):continue
        safe=b&~(g|wave(g))
        if safe:asc|=safe;n+=1
    return n,asc
def prepared(W):
    js=np.empty((V,4),dtype=np.int16);coeff=np.zeros((V,4),dtype=np.float64)
    for p,row in enumerate(ADJ):
        nb=[q for q in row if not W&B[q]] if not W&B[p] else []
        if not nb:nb=[p]
        js[p,:]=p;js[p,:len(nb)]=nb;coeff[p,:len(nb)]=1/len(nb)
    return js,coeff
def probabilities(graph,tg,speed,hh):
    js,coeff=graph;ids=list(cells(tg)); val=np.zeros(V);val[ids]=1.
    result={}
    for t in range(max(hh)+1):
        if t in hh:result[t]=val.copy()
        if t==max(hh):break
        for _ in range(speed):val=np.sum(val[js]*coeff,axis=1)
        val[ids]=1.
    return result
def verify():
    # One isolated four-cell strip, target leftmost. Hitting allowed at
    # turn endpoint only, so a pig cannot flip parity even if hitting mid-turn.
    w=ALL^bitmask(range(4));x=prepared(w)
    pp=probabilities(x,B[0],1,{0,1,2,3})
    assert pp[2][2]==.25 and pp[0][0]==1.
    pp=probabilities(x,B[0],2,{1,2})
    assert np.array_equal(pp[1][:4],[1,0,.25,0])
def run(panel,inp,wide,narrow):
    folder=P/(panel+'-map1'); rec=json.loads((folder/'recordings.json').read_text())
    assert len(rec)==(19 if panel=='practice' else 25)
    led={q['case']:q for q in csv.DictReader((R/'ahc008_terry_virtual_chase_absorb'/f'{panel}.native.csv').open())
        if q['arm']=='terry_independent'}
    details=[];counts=collections.Counter()
    for row in rec:
        case=row['case'];old=led[case]
        assert old['map']=='1' and all(row[x]==old[x] for x in ('input_sha','commands_sha'))
        f=folder/Path(case).with_suffix('.actions');raw=(inp/case).read_bytes()
        a=f.read_bytes();b=f.with_suffix('.trace').read_bytes()
        assert [sha(x)for x in (raw,a,b)]==[row[k]for k in ('input_sha','commands_sha','trace_sha')]
        actions=a.decode().splitlines();trace=b.decode().splitlines()
        assert replay(raw.decode(),trace,actions,detailed=True)[0]==int(old['score'])
        kinds,hist,_=observed(raw,trace);seen=set()
        for t,(ps,hs,ws) in enumerate(hist[:-1]):
            _,newh,neww=hist[t+1]
            before=bitmask(map(flat,ws));after=bitmask(map(flat,neww));delta=after^before
            if not delta:continue
            animals=list(map(flat,ps));h=list(map(flat,hs))
            pre=flood(bitmask(h),ALL^before)
            post=flood(bitmask(map(flat,newh)),ALL^after)
            hits=[]
            for i,g,room in wide:
                if g&~after or not g&delta or room&(after|post):continue
                if flood(room&-room,ALL^after)!=room:continue
                triggered=[j for j,p in enumerate(animals) if B[p]&room&pre and j not in seen]
                if not triggered:continue
                seen.update(triggered);counts['nine_body_events']+=1
                for j in triggered:
                    counts['caught_species_'+str(kinds[j])]+=1
                    if kinds[j]<=3:hits.append((i,j,kinds[j]))
            if not hits:continue
            n,tg=targets(before,h,narrow)
            stop=299-t;hh=set(H)|{min(x,stop) for x in H}
            graph=prepared(before)
            cache={k:probabilities(graph,tg,k,hh) for k in sorted({x[2] for x in hits})}
            for i,j,k in hits:
                value=lambda x:float(cache[k][x][animals[j]])
                d=dict(case=case,turn=t,room=i,id=j,species=k,narrow_rooms=n,target_cells=tg.bit_count(),
                       mode=('Chase' if old['mode']=='1' and t>=275 else 'Normal'),
                       uncapped={x:value(x) for x in H},capped={x:value(min(x,stop)) for x in H})
                details.append(d)
    assert len({(d['case'],d['id']) for d in details})==len(details)
    stat={}
    for typ in (0,1,2,3):
        col=[d for d in details if typ==0 or d['species']==typ]
        if not col:continue
        n=len(col);num=sum(d['capped'][25]>=.80 for d in col)
        stat[typ]=dict(n=n,good25=num,fraction25=num/n,
            by_h={k:{x:dict(mean=statistics.fmean(d[x][k]for d in col),median=statistics.median(d[x][k]for d in col),
                            at_least_80=sum(d[x][k]>=.80 for d in col),
                            zero=sum(d[x][k]==0 for d in col)) for x in ('uncapped','capped')} for k in H})
    return dict(boards=len(rec),counts=counts,by_species=stat,observations=details,passes=stat.get(0,{}).get('fraction25',0)>=.60)
if __name__=='__main__':
    verify();wide,narrow=layout()
    p=run('practice',R/'ahc008_focus_assets/tools/in',wide,narrow)
    i=run('fresh',R/'ahc008_terry_first_chase_reuse/default-fresh100/in',wide,narrow)
    print(json.dumps(dict(catalog_wide=len(wide),catalog_narrow=len(narrow),practice=p,fresh=i,passes=p['passes'] and i['passes']),indent=2))
