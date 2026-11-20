import sys
from pathlib import Path
solver=Path(sys.argv[1]).resolve()
import subprocess
import tempfile
import random
r=random.Random(136820)
def transpose(board): return list(map(''.join,zip(*board)))
def key(words,board):
 lines=[x+x for x in board+transpose(board)]
 return sum(any(w in line for line in lines) for w in words),sum(s.count('.') for s in board)
def known_words(board,r):
 lines=[s+s for s in board+transpose(board)]
 return [s[p:p+k] for s in lines for p in range(3) for k in range(2,4) if '.' not in s[p:p+k]]
with tempfile.TemporaryDirectory() as tmp:
 root=Path(tmp)
 ap,bp=root/'a',root/'b'
 wins=0
 for i in range(70):
  raw_a=[''.join(r.choices('ABCD.',k=3)) for _ in range(3)]
  raw_b=[''.join(r.choices('ABCD.',k=3)) for _ in range(3)]
  ap.write_text('\n'.join(raw_a)); bp.write_text('\n'.join(raw_b))
  w=known_words(raw_a,r)+known_words(raw_b,r)+['AB','BC','AC','CD','ABC']*3
  result=subprocess.run([solver,ap,bp],input=f'3 {len(w)}\n'+'\n'.join(w),text=True,check=True,capture_output=True)
  s=dict(t.split('=') for t in result.stderr.split())
  x,y,t=(int(s[v]) for v in 'xyt')
  b=[''.join(raw_b[(c+x)%3][(a+y)%3] if t else raw_b[(a+y)%3][(c+x)%3] for c in range(3)) for a in range(3)]
  differences=[(y,x) for y in range(3) for x in range(3) if raw_a[y][x]!=b[y][x]]
  best=(0,0)
  for bits in range(1<<len(differences)):
   board=list(map(list,raw_a))
   for j,(y,x) in enumerate(differences):
    if bits>>j&1: board[y][x]=b[y][x]
   best=max(best,key(w,list(map(''.join,board))))
  got=key(w,result.stdout.split())
  assert got==best and got[0]==int(s['predicted'])
  wins+=got>max(key(w,raw_a),key(w,raw_b))
 print('brute-force 70/70, strict lex parent upgrades',wins)
