from pathlib import Path
import hashlib,sys
p,out=map(Path,sys.argv[1:]);home=Path(__file__).resolve().parent
assert hashlib.sha256(p.read_bytes()).hexdigest()=='46f210cc5dd50b8a6088724861ae5c2b044d08edb109a891ee217352730fcb84'
s=p.read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,a[:120]
 s=s.replace(a,b)
rep(' int select(vector<bool> eligible, int excluded=-1){',' int select(vector<bool> eligible, int excluded=-1,int railPet=-1,const bitset<V>* protect=nullptr){')
rep('  for(auto p:pets)if(groups[group[p.p]])apos.push_back(p.p),at.push_back(p.t);','''  for(int j=0;j<int(pets.size());j++)if(j!=railPet&&groups[group[pets[j].p]])apos.push_back(pets[j].p),at.push_back(pets[j].t);''')
rep('   if(best<v)best=v,ret=i;','   if(best<v&&(!protect||jointPocketCert(z,railPet,*protect)))best=v,ret=i;')
rep('     if(v>best){keep=move(p);best=v;}','     if(v>best&&(!protect||jointPocketCert(p,railPet,*protect))){keep=move(p);best=v;}')
rep(' void pocketPlan(vector<bool> canWork, int &target,int &phase,int& bestPrev,int excluded){',' void pocketPlan(vector<bool> canWork, int &target,int &phase,int& bestPrev,int excluded,int railPet=-1,const bitset<V>* protect=nullptr){')
rep('   target=select(canWork,excluded);phase=turn;','   target=select(canWork,excluded,railPet,protect);phase=turn;')
rep(' string plan(){',(home/'joint.inc').read_text()+' string plan(){')
rep('  globalset.fill(0);globalmoved.fill(0);\n  int hm=','  globalset.fill(0);globalmoved.fill(0);\n  if(jointRescue())return act;\n  int hm=')
rep(' g.bankMode=5;', ''' g.bankMode=5;
 for(int i=1;i<argc;i++)if(string_view(argv[i])=="--no-joint-rescue")g.jointEnabled=false;''')
rep(' if(const char* p=getenv("AHC008_TERRY_LOG")){',''' if(const char* p=getenv("AHC008_JOINT_RESCUE_LOG")){
  ofstream f(p);f<<g.jointTurns<<' '<<g.jointRailTurns<<' '<<g.jointRailHeads<<' '<<g.jointPocketWalls<<'\\n';
 }
 if(const char* p=getenv("AHC008_TERRY_LOG")){''')
out.write_text(s)
