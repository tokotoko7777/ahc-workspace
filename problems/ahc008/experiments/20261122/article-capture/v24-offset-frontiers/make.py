from pathlib import Path
import hashlib,sys
root=Path(__file__).resolve().parent
src,out=map(Path,sys.argv[1:])
assert hashlib.sha256(src.read_bytes()).hexdigest()=='7ae8fc4c25a5b69c448682b62c428fb73f83b7a2d826003f07b2ea173be06639'
s=src.read_text()
a=s.index('namespace lastcap {')
b=s.index('namespace terry_maps',a)
m=s[a:b]
f=' Shape search(int turn,const Board& wall,const Board& main,int size,'
assert m.count(f)==1
m=m.replace(f,(root/'frontier.inc').read_text()+f)
old='   active=search(turn,wall,root,size,humans,pets,protect);'
assert m.count(old)==1
m=m.replace(old,'''   active=offsetFrontier(turn,wall,root,size,humans,pets,protect);
   activeOffset=active.value>=0;
   if(activeOffset)++offsetAttempts;
   else active=search(turn,wall,root,size,humans,pets,protect);''')
m=m.replace(' int attempts=0, actions=0,',' int offsetAttempts=0,offsetFinish=0; bool activeOffset=false;\n int attempts=0, actions=0,',1)
q='began=turn;deadline=std::min({300,turn+cfg.maxAttempt,turn+cfg.maxTotal-total});++attempts;'
assert m.count(q)==1
m=m.replace(q,q+'\n   if(activeOffset)deadline=std::min(deadline,turn+8);')
q='placed+=K;++closes;'
assert m.count(q)==1
m=m.replace(q,q+'if(activeOffset)++offsetFinish;')
s=s[:a]+m+s[b:]
point=' if(const char* p=getenv("AHC008_TERRY_LOG")){'
assert s.count(point)==1
s=s.replace(point,''' if(const char* p=getenv("AHC008_OFFSET_FRONTIER_LOG")){
  ofstream f(p);f<<g.finalCapture.offsetAttempts<<' '<<g.finalCapture.offsetFinish<<'\\n';
 }
'''+point)
out.write_text(s)
