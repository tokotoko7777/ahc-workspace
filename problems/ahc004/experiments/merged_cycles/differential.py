from merge import *
import subprocess,time
t=time.perf_counter()
for L in range(5,9):
 for j in range(12):
  _,r=make_case(L,400+30*j,190217+37*j)
  for k in (5,6,7):
   got=subprocess.run(['./driver'],input=f'20 {len(r)} {k}\n'+'\n'.join(r)+'\n',text=True,capture_output=True,check=True).stdout.split()
   expected=make_patterns(r,k)
   assert set(got)==expected,(L,j,k,len(got),len(expected),set(got)^expected)
print('144 cross-checks:',time.perf_counter()-t,'s')
