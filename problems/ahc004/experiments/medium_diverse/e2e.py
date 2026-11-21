from partial import *
import sys
import time, subprocess
control,candidate=sys.argv[1:]
for L,M,seed in [(6,600,7_904_200_730+j) for j in range(4)] + [(6,640,7_904_200_835+j)for j in range(4)] + [(6,670,7_904_200_555+j)for j in range(4)] + [(6,600,6_030_501_213+6*120+4),(6,650,6_030_501_213+6*120+4)]:
    _,w=make_case(L,M,seed)
    a=[]
    for exe in [control,candidate]:
        t=time.monotonic()
        o=subprocess.run([exe],input=f'20 {len(w)}\n'+'\n'.join(w)+'\n',
                         stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,check=True,timeout=3.05)
        dt=time.monotonic()-t
        b=o.stdout.split();assert len(b)==20 and all(len(s)==20 for s in b)
        assert all(set(s) <= set('ABCDEFGH.') for s in b)
        lines=[s+s[:-1] for s in b+list(map(''.join,zip(*b)))]
        hits=sum(any(s in z for z in lines)for s in w)
        line=next((l for l in o.stderr.splitlines() if l.startswith('medium_triples=')),'')
        a.append((hits,round(dt,3),line))
    print(L,M,seed,a,flush=True)
