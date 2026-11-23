"""Read-only detached same-turn occupancy/capture census on signed traces."""
from pathlib import Path
from collections import Counter,defaultdict
import sys, json
D=Path(__file__).resolve().parent;R=D.parent
sys.path.insert(0,str(R/'ahc008_terry_inward_gates'))
from screen import ALL, B,bitmask,cells,flat,flood,markers,near,observed,replay,wave
sys.path.insert(0,str(R/'ahc008_terry_hunt_wall_audit'))
from analyze import sha
ARCH=R/'ahc008_terry_hunt_wall_audit'
INPUT=R/'ahc008_terry_incoming_shadow/held100/in'
NBS=[wave(b)for b in B]
C=R/'ahc008_terry_small_rooms/catalog.tsv'
def options():
 by=defaultdict(list)
 for line in C.read_text().splitlines():
  m,i,l,r,*a=map(int,line.split())
  w,_=markers()[m]
  doors=[x for x in (l,r)if x>=0];gates=bitmask(doors)
  body=flood(B[min(a)],ALL^(w|gates))
  if body!=bitmask(a) or body.bit_count()>4:continue
  must=wave(body)&~(body|gates)
  assert not(must&~w)
  by[m].append((i,doors,gates,body,must,gates|wave(gates)))
 return by

def district(f,ps,hh):
 op=ALL^f;ex=flood(bitmask(hh),op)
 assert all(not (ex&B[p]) for p in ps)
 ct={}
 for a,p in enumerate(ps):
  dest=flood(B[p],op)
  ct[dest]=ct.get(dest,0)|(1<<a)
 return ct

def realize(ps,hh,ww,row,cmd,pnext,hn,wn):
 i,doors,gates,body,must,veto=row
 if any(NBS[q]&body==0 for q in doors):raise ValueError("closed bogus gate")
 person=bitmask(hh);pet=bitmask(ps)
 if gates&ww or must&~ww or person&(body|gates) or pet&veto:return (0,0)
 op=ALL^ww; dp=op&~gates
 post_cache={}
 for h in set(hh):
  before=flood(B[h],op)
  if not before&body:return (0,0)
  after=flood(B[h],dp)
  if before&~(body|gates|after):return (0,0)
  post_cache[h]=after
 assert flood(body&-body,dp)==body
 # Any suitable adjacent exterior pose, not only the axial catalog stance.
 def adjacent(g,h):
  return bool(B[h]&NBS[g])
 def permits(g,h,c):
  return c=='.' or c.islower() and near(h,c.upper())==g
 candidates=[[i for i,h in enumerate(hh)if adjacent(g,h)]for g in doors]
 for actor in __import__('itertools').product(*candidates):
  if len(set(actor))!=len(doors):continue
  found=True
  if not all(permits(g,hh[j],cmd[j]) for g,j in zip(doors,actor)):
   continue
  # Replay original simultaneous commands for the unmodified people.
  # Old exact move destinations remain legal only if outside new doors.
  if any(cmd[j].isupper() and B[hn[j]]&gates for j in range(len(hh))if j not in actor):continue
  nh=[hh[j] if j in actor else hn[j] for j in range(len(hh))]
  if bitmask(nh)&(body|gates):continue
  ow=ALL^wn; alt=ow&~gates
  if flood(body&-body,alt)!=body:continue
  if any(flood(B[h],ow)&~(body|gates|flood(B[h],alt)) for h in set(nh)):continue
  return (1,1)
 return (int(any(len(set(z))==len(doors)for z in __import__('itertools').product(*candidates))),0)

def board(r,rooms):
 name=r['case'];inp=INPUT/name;p=(ARCH/'tapes'/name).with_suffix('.actions');t=p.with_suffix('.trace')
 raw=inp.read_bytes();actions=p.read_text().splitlines();tape=t.read_text().splitlines()
 assert (sha(inp),sha(p),sha(t))==(r['input_sha'],r['commands_sha'],r['trace_sha'])
 assert len(actions)==300 and actions==tape[::2]
 score,*_=replay(raw.decode(),tape,actions,detailed=True)
 assert int(score)==int(r['score'])
 kinds,hist,_=observed(raw,tape)
 H=[([flat(p)for p in ps],[flat(h)for h in hs],bitmask(map(flat,w)))for ps,hs,w in hist]
 if not r['actual_full']:return None
 pp,hh,w=H[-1];cts=district(w,pp,hh)
 groups=list(cts.items())
 col=[j for i in range(len(pp))for j,(b,aa)in enumerate(groups)if aa>>i&1]
 assert len(col)==len(pp)
 ctr=Counter();uniq=[set(),set(),set()]; credit=[set(),set(),set()]
 for turn in range(100,275):
  ps,ph,pw=H[turn]; pnext,hn,wn=H[turn+1]
  c=actions[turn]
  for i,doors,gates,body,must,veto in rooms:
   if gates&pw or must&~pw:continue
   heads=[i for i,p in enumerate(ps)if B[p]&body]
   if len(heads)<2 or len({col[i]for i in heads})<2:continue
   human=bitmask(ph);pet=bitmask(ps)
   if human&(body|gates) or pet&veto:continue
   old=ALL^pw;new=old&~gates
   if not flood(human,old)&body:continue
   if any(flood(B[h],old)&~(body|gates|flood(B[h],new))for h in set(ph)):continue
   assert flood(body&-body,new)==body
   nh=bitmask(heads)
   subset={j for j,(b,hh)in enumerate(groups)if nh&hh==hh and not b&body}
   staffed,compatible=realize(ps,ph,pw,(i,doors,gates,body,must,veto),c,pnext,hn,wn)
   key=(i,nh)
   for k,yes in enumerate((1,staffed,compatible)):
    if yes:
     ctr[k]+=1;uniq[k].add(key);credit[k]|=subset
 return dict(events=[ctr[k]for k in range(3)],distinct=[len(x)for x in uniq],
             credit=[sum(groups[j][0].bit_count()for j in a)for a in credit])

def main():
 assert sha(ARCH/"recordings.json")=="dde3b2186e2c5d94b46623bda51b37206fb458cec92787ca5b939793531c25c5"
 assert sha(C)=="2d697e1b2cbf0cd9d754d38a18059e2fbcd484b7440868da8016f8310132ba06"
 rows=json.loads((ARCH/'recordings.json').read_text())
 assert len(rows)==100 and sum(r['actual_full'] for r in rows)==99
 options_by_map=options()
 report=[a for r in rows if (a:=board(r,options_by_map[r['map']])) is not None]
 assert len(report)==99
 result={k:[sum(a[k][j]for a in report)for j in range(3)]for k in ('events','distinct','credit')}
 result['boards']=[sum(a['distinct'][j]>0 for a in report)for j in range(3)]
 result['pass']=result['credit'][0]>=200 and result['distinct'][1]>=10 and result['boards'][1]>=5
 result['maps']={m:len(opts)for m,opts in options_by_map.items()}
 print(json.dumps(result,indent=2))
if __name__=='__main__':main()
