import pathlib as P, sys, subprocess as sp, hashlib, time, re
home=P.Path.cwd()
tools_arg,tag,n,binary,*arg=sys.argv[1:]; tools=P.Path(tools_arg).resolve(); n=int(n)
cwd=home/tag; cwd.mkdir(exist_ok=True)
if not (cwd/'in').exists():
 (cwd/'seeds').write_text(''.join(str(int.from_bytes(hashlib.sha256(f'ahc008/rails/202611/{tag}/{i}'.encode()).digest()[:8],'little'))+'\n' for i in range(200)))
 sp.run([tools/'gen','seeds'],cwd=cwd,check=True)
scores=[]
for j,f in enumerate(sorted((cwd/'in').glob('*.txt'))[:n]):
 t=time.monotonic(); r=sp.run([tools/'tester',home/binary,*arg],input=f.read_bytes(),capture_output=True,timeout=5)
 r.check_returncode()
 assert len(r.stdout.splitlines())==300
 score=int(re.findall(rb'Score = (\d+)',r.stderr)[-1])
 if not score: raise RuntimeError(f'{f}, {r.stderr}')
 diag=re.findall(rb'rails (.*)',r.stderr)
 print(f'{j},{score},{time.monotonic()-t:.6f}'+((','+diag[0].decode()) if diag else ''),flush=True)
 scores.append(score)
print(binary,arg,'n',len(scores),'sum',sum(scores),'avg',sum(scores)/len(scores),file=sys.stderr)
