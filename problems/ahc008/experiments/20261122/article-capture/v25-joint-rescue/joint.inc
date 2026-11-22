 bool jointEnabled=true;
 int jointTurns=0,jointRailHeads=0,jointRailTurns=0,jointPocketWalls=0;
 // Full proposed preparation must leave all humans together outside,
 // leave the protected rail intact, and separate a different real pet.
 bool jointPocketCert(const Pen& z,int railPet,const bitset<V>& protectedRail)const{
  bitset<V> end;
  for(int p=0;p<V;p++)if(wall[p])end[p]=true;
  int n=0;
  for(int p:z.wall)if(!wall[p]){
   if(protectedRail[p])return false;
   end[p]=true;++n;
  }
  if(!n||n>8)return false;
  bitset<V> good;
  int q[V],k=1;q[0]=humans[0];if(end[q[0]])return false;good[q[0]]=true;
  for(int i=0;i<k;i++)for(int p:nb[q[i]])if(p>=0&&!end[p]&&!good[p])good[p]=true,q[k++]=p;
  if(k<300||4*k<3*currentCt[group[humans[0]]])return false;
  for(int p:humans)if(!good[p])return false;
  if((protectedRail&~good).any())return false;
  if(!good[pets[railPet].p])return false;
  for(int pi=0;pi<int(pets.size());pi++)if(pi!=railPet&&
     group[pets[pi].p]==group[humans[0]]&&!good[pets[pi].p]&&!end[pets[pi].p])return true;
  return false;
 }
 bool jointRescue(){
  if(!jointEnabled||humans.size()!=5||turn<160||turn>=205||
     turn<bankRelease||lineId>=0||overBudget())return false;
  const int root=group[humans[0]];
  int pop=0;for(auto p:pets)pop+=group[p.p]==root;
  if(pop<=2)return false;
  for(int i=0;i<5;i++)if(captiveHuman(i)||group[humans[i]]!=root)return false;
  bitset<V> closed;array<int,V> body;body.fill(-1);
  for(int p=0;p<V;p++)if(wall[p])closed[p]=true;
  vector<int> ready(banks.size());
  for(int k=0;k<int(banks.size());k++){
   auto& x=banks[k];
   ready[k]=all_of(x.side.begin(),x.side.end(),[&](int p){return wall[p];})&&
     !wall[x.g]&&!wall[x.k]&&none_of(x.inner.begin(),x.inner.end(),[&](int p){return wall[p];});
   if(ready[k])for(int p:x.inner)body[p]=k;
  }
  array<BankPath,5> paths;
  for(int i=0;i<5;i++)paths[i].run(humans[i],closed,body);
  vector<terry_lane::Tube> tr;
  for(auto& z:banks)tr.push_back({z.l,z.h,z.g,z.k,z.inner});
  terry_lane::Graph graph;
  graph.rebuild(nb,wall,tr,ready);
  vector<Forecast> forecast(pets.size());
  for(int p=0;p<int(pets.size());p++)if(group[pets[p].p]==root)forecastPet(p,3,forecast[p]);
  struct Trial{double value;int tube,a,b,pet,d;};
  vector<Trial> trials;
  for(int k=0;k<int(banks.size());k++)if(ready[k]&&!graph.bridge[k]){
   auto& x=banks[k];if(any_of(humans.begin(),humans.end(),[&](int p){return body[p]==k;}))continue;
   for(int a=0;a<5;a++) for(int b=0;b<5;b++)if(a!=b){
    int da=paths[a].d[x.l],db=paths[b].d[x.h],d=max(da,db);
    if(min(da,db)<0||d>3)continue;
    int selected=-1;double value=0.;
    for(int pi=0;pi<int(pets.size());pi++)if(group[pets[pi].p]==root){
     const int p=pets[pi].p;
     double z;
     if(d==0)z=(!block[x.g]&&!block[x.k]&&(p==x.inner[1]||p==x.inner[2]))?2.0:0.;
     else z=(forecast[pi][d][x.inner[1]]+forecast[pi][d][x.inner[2]])/(1+d);
     if(z>value)value=z,selected=pi;
    }
    if(selected>=0&&value>=.035)trials.push_back({value,k,a,b,selected,d});
   }
  }
  sort(trials.begin(),trials.end(),[](auto& a,auto& b){
   return tuple(-a.value,a.d,a.tube,a.a,a.b,a.pet)<tuple(-b.value,b.d,b.tube,b.a,b.b,b.pet);
  });
  int tries=0;set<tuple<int,int,int>> tested;
  const auto oldAct=act;const auto oldSet=globalset,oldMoved=globalmoved,oldNeed=need,oldAvoid=avoid;
  const int original=pockets.size();
  for(auto v:trials){
   if(tries>=3||overBudget())break;
   if(!tested.emplace(v.tube,v.pet,(1<<v.a)|(1<<v.b)).second)continue;
   ++tries;
   auto& x=banks[v.tube];
   bitset<V> protectedRail;
   for(int p:{x.l,x.h,x.g,x.k})protectedRail[p]=true;
   for(int p:x.inner)protectedRail[p]=true;
   int nextA=paths[v.a].step[x.l],nextB=paths[v.b].step[x.h];
   if(nextA>=0)protectedRail[nextA]=true;
   if(nextB>=0)protectedRail[nextB]=true;
   vector<bool> crew(5,true);crew[v.a]=crew[v.b]=false;
   int p=select(crew,-1,v.pet,&protectedRail);
   if(p<0)continue;
   int seq=turn,remain=0;
   for(int q:pockets[p].wall)remain+=!wall[q];
   for(int q=0;q<V;q++)if(protectedRail[q])globalmoved[q]=1;
   // Start from the actual three-person mission, never dispatch all five
   // first and replace two commands afterwards.
   pocketPlan(crew,p,seq,remain,-1,v.pet,&protectedRail);
   if(p>=0&&act!=oldAct){
    bool legal=true;
    if(v.d==0){
     legal=!globalset[x.g]&&!globalset[x.k]&&!block[x.g]&&!block[x.k];
     for(int i=0;i<5;i++)if(crew[i]&&isupper(act[i])){
      int point=nb[humans[i]][find(MOVE,MOVE+4,act[i])-MOVE];
      if(point==x.g||point==x.k||find(x.inner.begin(),x.inner.end(),point)!=x.inner.end())legal=false;
     }
     if(legal){
      act[v.a]=BUILD[dir(x.l,x.g)];act[v.b]=BUILD[dir(x.h,x.k)];
     }
    }else{
     if(nextA>=0&&globalset[nextA])legal=false;
     if(nextB>=0&&globalset[nextB])legal=false;
     if(legal){
      if(nextA>=0)act[v.a]=MOVE[dir(humans[v.a],nextA)];
      if(nextB>=0)act[v.b]=MOVE[dir(humans[v.b],nextB)];
     }
    }
    if(legal){
     bitset<V> after=closed,seen;vector<int> pos=humans;
     for(int i=0;i<5;i++)if(islower(act[i])){
      int p=nb[humans[i]][find(BUILD,BUILD+4,act[i])-BUILD];
      if(p<0||after[p]||block[p])legal=false;
      else after[p]=true;
     }else if(isupper(act[i])){
      int p=nb[humans[i]][find(MOVE,MOVE+4,act[i])-MOVE];
      if(p<0)legal=false;
      else pos[i]=p;
     }
     for(int p:pos)if(after[p])legal=false;
     if(legal){
      int q[V],n=1;q[0]=pos[0];seen[q[0]]=true;
      for(int i=0;i<n;i++)for(int j:nb[q[i]])if(j>=0&&!after[j]&&!seen[j])seen[j]=true,q[n++]=j;
      for(int p:pos)if(!seen[p])legal=false;
      if(n<300||4*n<3*currentCt[root])legal=false;
      if(v.d==0)for(int p:x.inner)if(seen[p])legal=false;
     }
    }
    if(legal){
     ++jointTurns;
     if(v.d==0){
      ++jointRailTurns;
      for(auto q:pets)jointRailHeads+=q.p==x.inner[1]||q.p==x.inner[2];
     }
     for(int i=0;i<5;i++)if(crew[i]&&islower(act[i]))++jointPocketWalls;
     pockets.resize(original);return true;
    }
   }
   act=oldAct;globalset=oldSet;globalmoved=oldMoved;need=oldNeed;avoid=oldAvoid;
   pockets.resize(original);
  }
  return false;
 }
