from partial import *
import subprocess
for L in range(5,9):
 for M in [450,650,800]:
  for k in [5,6]:
   ret=[]
   for i in range(40):
    lines,w=make_case(L,M,2_402_100_019+L*1000+i)
    r=subprocess.run(['./aug-driver'],input=f'{M} {k}\n'+'\n'.join(w)+'\n',text=True,capture_output=True,check=True).stdout.splitlines()
    t=float(r[0])
    case=collections.Counter(w)
    canon=lambda p:min(p[i:]+p[:i] for i in range(20))
    truth={canon(l[:20]) for l in lines}
    a={s[2:] for s in r if s[:2]=='O '}
    b={s[2:] for s in r if s[:2]=='P '}
    def score(s):
      wr=s+s[:-1]
      return sum(case[w]*0+min(3,case[w])*(len(w)-1)**2 for w in case if w in wr)+(s in a)*5
    def choose(s):
      out=sorted(s,key=lambda s:(-score(s),s))[:40]
      covers=set()
      for w in out:
        wr=w+w[:-1]
        covers|={z for z in case if z in wr}
      n=sum(case[z] for z in covers)
      return len(out)>=28 and n>=.75*M,len(truth&set(out))
    ga,ta=choose(a)
    gj,tj=choose(a|b)
    ret.append((len(a&truth),len(a-truth),len(b&truth),len(b-truth),len((b-a)&truth),len((b-a)-truth),ga,gj,not ga and gj,ta,tj,t))
   v=list(map(statistics.mean,zip(*ret)))
   print(f'{L=} {M=} {k=} O true/false {v[0]:.1f}/{v[1]:.1f} P true/false {v[2]:.1f}/{v[3]:.1f} novel true/false {v[4]:.1f}/{v[5]:.1f} gates old/new/new-only {v[6]*40:.0f}/{v[7]*40:.0f}/{v[8]*40:.0f} final true {v[9]:.1f}/{v[10]:.1f} assembly ms mean,max {v[11]:.1f},{max(x[-1] for x in ret):.1f}',flush=True)
