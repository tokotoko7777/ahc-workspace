from pathlib import Path
import hashlib,sys
p,o=map(Path,sys.argv[1:])
data=p.read_bytes()
assert hashlib.sha256(data).hexdigest()=='644307bc102fdd9f815960c79c6ebd9cb69e0a817f6398b3fed7e96eb7cbab43'
s=data.decode()
def rep(a,b):
 global s
 assert s.count(a)==1,a[:120]
 s=s.replace(a,b)
rep(' bool jointEnabled=true;',Path(__file__).with_name('stable.inc').read_text()+' bool jointEnabled=true;')
rep('const bitset<V>& protectedRail)const{','const bitset<V>& protectedRail,int fixedPet=-1)const{')
rep('for(int pi=0;pi<int(pets.size());pi++)if(pi!=railPet&&','for(int pi=0;pi<int(pets.size());pi++)if(pi!=railPet&&(fixedPet<0||pi==fixedPet)&&')
rep('  if(!jointEnabled||humans.size()!=5||turn<160||turn>=205||','''  auto holding=std::exchange(stableJoint,StableJoint{});
  const bool resume=stableJointEnabled&&holding.last+1==turn;
  if(!jointEnabled||humans.size()!=5||turn<160||turn>=205||''')
rep('  int tries=0;set<tuple<int,int,int>> tested;','''  if(resume){
   erase_if(trials,[&](const Trial& q){
    return tie(q.tube,q.a,q.b,q.pet)!=tie(holding.tube,holding.a,holding.b,holding.rail);
   });
  }
  int tries=0;set<tuple<int,int,int>> tested;''')
rep('   int p=select(crew,-1,v.pet,&protectedRail);','''   int p;
   if(resume){
    if(!jointPocketCert(holding.pen,v.pet,protectedRail,holding.pet))break;
    p=pockets.size();
    pockets.push_back(holding.pen);
   }else p=select(crew,-1,v.pet,&protectedRail);''')
rep('   pocketPlan(crew,p,seq,remain,-1,v.pet,&protectedRail);\n   if(p>=0&&act!=oldAct){','''   const int identity=p;
   pocketPlan(crew,p,seq,remain,-1,v.pet,&protectedRail);
   if(p>=0&&(!resume||p==identity)&&act!=oldAct){''')
rep('    if(legal){\n     ++jointTurns;', '''    StableJoint pending;
    if(legal&&stableJointEnabled){
     const auto& z=pockets[p];
     bitset<V> ended=closed;
     for(int q:z.wall)ended[q]=true;
     vector<int> realBuild;
     bool walk=false;
     for(int i=0;i<5;i++)if(crew[i]&&islower(act[i])){
      int q=nb[humans[i]][find(BUILD,BUILD+4,act[i])-BUILD];
      if(ended[q]&&!wall[q])realBuild.push_back(q);
     }
     if(resume){
      bool seenProgress=any_of(holding.issued.begin(),holding.issued.end(),[&](int q){return wall[q];});
      pending.dry=seenProgress?0:holding.dry+1;
      // Crew routes may not start into a reserved wall or into a projected pen.
      bitset<V> retained;
      int q[V],n=1;q[0]=humans[0];retained[q[0]]=true;
      for(int k=0;k<n;k++)for(int u:nb[q[k]])if(u>=0&&!ended[u]&&!retained[u])
       retained[u]=true,q[n++]=u;
      for(int i=0;i<5;i++)if(isupper(act[i])){
       int to=nb[humans[i]][find(MOVE,MOVE+4,act[i])-MOVE];
       if(to<0||!retained[to])legal=false;
      }
      if(realBuild.empty())walk=stableJointWalk(ended,crew,z);
      if(pending.dry>2||(realBuild.empty()&&!walk))legal=false;
     }
     // Never rewrite the original first accepted turn, and never reuse
     // a failed held hypothesis as a different pocket on this turn.
     if(legal&&v.d>0){
      int targetPet=resume?holding.pet:-1;
      if(targetPet<0)for(int j=0;j<int(pets.size());j++)
       if(j!=v.pet&&jointPocketCert(z,v.pet,protectedRail,j)){targetPet=j;break;}
      if(targetPet>=0){
       pending.last=turn;pending.tube=v.tube;pending.a=v.a;pending.b=v.b;
       pending.rail=v.pet;pending.pet=targetPet;pending.pen=z;
       pending.issued=move(realBuild);
      }
     }
    }
    if(legal){
     if(stableJointEnabled)stableJoint=move(pending);
     if(resume)++stableContinued;
     ++jointTurns;''')
rep(' if(const char* p=getenv("AHC008_JOINT_RESCUE_LOG")){',''' if(const char* p=getenv("AHC008_STABLE_JOINT_LOG")){
  ofstream f(p);f<<g.stableContinued<<'\\n';
 }
 if(const char* p=getenv("AHC008_JOINT_RESCUE_LOG")){''')
rep(' for(int i=1;i<argc;i++)if(string_view(argv[i])=="--no-joint-rescue")g.jointEnabled=false;', ''' for(int i=1;i<argc;i++){
  if(string_view(argv[i])=="--no-joint-rescue")g.jointEnabled=false;
  if(string_view(argv[i])=="--no-stable-joint")g.stableJointEnabled=false;
 }''')
o.write_text(s)
