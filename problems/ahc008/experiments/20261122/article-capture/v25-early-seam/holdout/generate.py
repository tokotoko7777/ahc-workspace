from pathlib import Path
import subprocess,hashlib,json,sys
p=Path(__file__).resolve().parent
dst=p/'in';src=p/'candidate'
src.mkdir();dst.mkdir()
seedfile=p/'candidate-seeds.txt'
s=seedfile.read_text().splitlines()
subprocess.run([str(Path(sys.argv[1]).resolve()),str(seedfile)],cwd=src,check=True)
manifest=[]
for i,seed in enumerate(s):
 f=src/'in'/f'{i:04}.txt';b=f.read_bytes();q=b.split()
 if int(q[3*int(q[0])+1])!=5:continue
 target=f'{len(manifest):04}.txt'
 (dst/target).write_bytes(b)
 manifest.append(dict(case=target,seed=int(seed),stream_index=i,sha256=hashlib.sha256(b).hexdigest()))
 if len(manifest)==100:break
assert len(manifest)==100
(p/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
