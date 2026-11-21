from partial import *
from pathlib import Path
import time,subprocess
import sys
control,candidate=sys.argv[1:]
cases=[(L,M,8_370_337_042+j+1000*L+M*23) for L,M in [(6,720),(6,800),(7,500),(7,680)] for j in range(5)]
for L,M,seed in cases:
    _,w=make_case(L,M,seed)
    a=[]
    for exe in [control,candidate]:
        t=time.monotonic()
        o=subprocess.run([exe],input=f'20 {len(w)}\n'+'\n'.join(w)+'\n',
                         stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,check=True,timeout=3.05)
        dt=time.monotonic()-t
        b=o.stdout.split();assert len(b)==20 and all(len(s)==20 and set(s)<=set('ABCDEFGH.') for s in b)
        lines=[s+s[:-1] for s in b+list(map(''.join,zip(*b)))]
        hits=sum(any(s in z for z in lines)for s in w)
        line='; '.join(l for l in o.stderr.splitlines() if l.startswith(('wild_lines=','disjoint_triples=')))
        a.append((hits,round(dt,3),line))
    print(L,M,seed,a,flush=True)
