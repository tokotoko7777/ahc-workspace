"""Actual prepared tube/two-stance geometry and capture time from exact replay."""
import json
from bridge import parse_geometry
from stochastic_lock_audit import B,idx,wave
from parallel_banks_anchored_cut_audit import buf
def measure(hist,geo,wavefile):
    rot,shift,tubes=parse_geometry(geo)
    st=[(list(map(idx,p)),list(map(idx,h)),buf(map(idx,w))) for p,h,w in hist]
    bank_n=1+max(x.bank for x in tubes)
    first_ready=[-1]*bank_n;staff_after_ready=[-1]*bank_n;first_lock=[-1]*bank_n
    milestones={}
    for t,(p,h,w) in enumerate(st):
        pp=buf(p);hh=buf(h)
        block=pp|wave(pp)|hh
        counts=[0]*bank_n;doors=[];staff_t=0;legal=0;people=set()
        for x in tubes:
            if x.wall&w!=x.wall:continue
            counts[x.bank]+=1
            if (x.gates|x.body)&w:continue
            doors.append(x)
            a=x.l in h and x.r in h and not x.body&hh
            if a:
                staff_t+=1
                if counts[x.bank]>=5 or first_ready[x.bank]>=0:
                    if staff_after_ready[x.bank]<0:staff_after_ready[x.bank]=t
            for i,y in enumerate(h):
                if y in (x.l,x.r):people.add(i)
            good=bool(x.body&pp) and not x.body&hh and not x.gates&block
            if good:legal+=1
            if a and good and t<300 and st[t+1][2]&x.gates==x.gates:
                if first_lock[x.bank]<0:first_lock[x.bank]=t+1
        for i,a in enumerate(counts):
            if first_ready[i]<0 and a>=5:
                first_ready[i]=t
                if staff_after_ready[i]<0 and any(y.bank==i and y.l in h and y.r in h and not y.body&hh for y in doors):
                    staff_after_ready[i]=t
        if t in (50,75,100):
            milestones[t]=dict(shells=sum(counts),open_tubes=len(doors),staffed_tubes=staff_t,
               available_workers=len(people),ready_banks=sum(a>=5 for a in counts),legal_tubes=legal)
    res=dict(ready=first_ready,staff=staff_after_ready,first_lock=first_lock,milestones=milestones)
    if wavefile.exists():
        head,*rows=wavefile.read_text().splitlines()
        nom,deadline=map(int,head.split());trace=[]
        for r in rows:
            t,k,b,a,x,y=map(int,r.split());trace.append([t,k,b,a,x,y])
            if k==1:
                assert t<=deadline and x in st[t][1] and y in st[t][1] and a==24
            if k==2:assert t==deadline and a<=24
        res['waves']=[nom,deadline,trace]
    return json.dumps(res,separators=(',',':'))
