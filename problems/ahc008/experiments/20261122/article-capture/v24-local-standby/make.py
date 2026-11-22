import hashlib,sys
from pathlib import Path
home=Path(__file__).resolve().parent
p,out=map(Path,sys.argv[1:])
assert hashlib.sha256(p.read_bytes()).hexdigest()=='7ae8fc4c25a5b69c448682b62c428fb73f83b7a2d826003f07b2ea173be06639'
s=p.read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
rep(' void parkUnassigned(', (home/'standby.inc').read_text()+' void parkUnassigned(')
a='  struct Park{int goal=-1,tube=-1;double value=0.;};'
rep(a,'  localStandby(ready,usedPeople,build,moveTo);\n'+a)
rep(' g.bankMode=5;', ''' g.bankMode=5;
 for(int i=1;i<argc;i++)if(string_view(argv[i])=="--no-local-standby")g.localStandbyEnabled=false;''')
a=' if(const char* p=getenv("AHC008_TERRY_LOG")){'
rep(a,''' if(const char* p=getenv("AHC008_LOCAL_STANDBY_LOG")){
  ofstream f(p);f<<g.localStandbyMatched<<' '<<g.localStandbyMoves<<'\\n';
 }
'''+a)
out.write_text(s)
