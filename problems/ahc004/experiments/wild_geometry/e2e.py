from partial import *
import sys
import time, subprocess
control, candidate = sys.argv[1:]
for L,M in [(6,680),(6,740),(6,800),(7,450),(7,550)]:
    for j in range(3):
        _,w=make_case(L,M,3_193_777_029+L*120+j)
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
            line=next((l for l in o.stderr.splitlines() if l.startswith('wild_lines=')),'')
            a.append((hits,round(dt,3),line))
        print(L,M,j,a,flush=True)
