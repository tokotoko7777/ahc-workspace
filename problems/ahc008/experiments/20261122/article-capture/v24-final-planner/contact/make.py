from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
a=s.index('namespace lastcap {')
b=s.index('namespace terry_maps::weights',a)
t=s[a:b]
def patch(p,q):
 global t
 assert t.count(p)==1,p
 t=t.replace(p,q)
patch('const std::vector<Pet>& pets,const Board& root){','const std::vector<Pet>& pets,const Board& root,bool keepOccupiedGate=false){')
patch('  z.trapped=0;\n  for(auto a:pets)if(root[a.p]){','  z.trapped=0;\n  bool usingMouth=false;\n  for(auto a:pets)if(root[a.p]){')
patch('''   if(std::find(z.cut.begin(),z.cut.end(),a.p)!=z.cut.end())return false;
   z.trapped+=!z.can[a.p];
  }
  if(!z.trapped)return false;''','''   if(std::find(z.cut.begin(),z.cut.end(),a.p)!=z.cut.end()){
    if(!keepOccupiedGate)return false;
    usingMouth=true;
    continue; // legal commands still cannot close the occupied mouth.
   }
   z.trapped+=!z.can[a.p];
  }
  if(!z.trapped&&!usingMouth)return false;''')
patch('!certify(active,wall,humans,pets,root)))','!certify(active,wall,humans,pets,root,humans.size()==5)))')
Path(sys.argv[2]).write_text(s[:a]+t+s[b:])
