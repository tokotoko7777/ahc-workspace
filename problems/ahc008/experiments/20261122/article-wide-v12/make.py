from pathlib import Path
root=Path(__file__).resolve().parent
s=(root.parent/'ahc008_integrated_v11/integrated.cpp').read_text()
def rep(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
rep(' array<int,300> integratedMode{},integratedReady{},integratedLocks{};', ''' array<int,300> integratedMode{},integratedReady{},integratedLocks{};
 array<array<int,3>,4> demandSeen{};
 array<array<int,5>,300> demandReport{};''')
rep('  const bool scheduled=turn>=35&&*max_element(local.begin(),local.end())>=5;\n  const bool partial=3*total<2*n && turn<101;', '''  array<array<bool,3>,4> active{};
  array<int,4> anchor{};
  array<double,4> anchorValue{};anchorValue.fill(-1);
  for(int k=0;k<n;k++){
   const auto& x=banks[k]; const int zone=x.r/10;
   if(wall[x.g]||wall[x.k])continue;
   double interest=0.;
   for(Pet p:pets)if(service[group[p.p]]){
    const int d=min(md(p.p,x.inner[1]),md(p.p,x.inner[2]));
    if(p.t<=3&&d<=4)demandSeen[x.b][zone]=turn+1;
    interest+=(p.t<=3?1.:.5)/(1+d);
   }
   // Nearest distinct people; if nearest same worker use either runner-up.
   array<pair<int,int>,2> left{{{99,-1},{99,-2}}},right=left;
   auto add=[](auto& t,int d,int i){
    if(d<0)return;
    if(d<t[0].first)t[1]=t[0],t[0]={d,i};
    else if(d<t[1].first)t[1]={d,i};
   };
   for(int i=0;i<m;i++)if(!usedPeople[i]){
    add(left,workers[i].bfs.distance(x.l),i);
    add(right,workers[i].bfs.distance(x.h),i);
   }
   int eta=100;
   for(auto [l,a]:left)for(auto [r,b]:right)if(a>=0&&b>=0&&a!=b)eta=min(eta,max(l,r));
   CGFloat_UNUSED
   const double value=interest/(1.+.10*eta);
   if(anchorValue[x.b]<value)anchorValue[x.b]=value,anchor[x.b]=zone;
  }
  for(int b=0;b<bankN;b++){
   for(int z=0;z<3;z++)
    active[b][z]=z==anchor[b]||(demandSeen[b][z]&&turn+1-demandSeen[b][z]<8);
  }
  int activeStock=0,activeReady=0,activeMask=0,fullBanks=0;
  array<int,4> activeN{},activeDone{};
  bitset<V> activeWalls;
  for(int k=0;k<n;k++){
   const auto& x=banks[k];
   if(active[x.b][x.r/10]){
    ++activeStock;activeReady+=ready[k];++activeN[x.b];activeDone[x.b]+=ready[k];
    for(int p:x.side)activeWalls[p]=true;
   }
  }
  for(int b=0;b<bankN;b++){
   for(int z=0;z<3;z++)if(active[b][z])activeMask|=1<<(3*b+z);
   if(3*activeDone[b]>=2*activeN[b])fullBanks|=1<<b;
  }
  const bool scheduled=turn>=35&&*max_element(local.begin(),local.end())>=5;
  const bool partial=3*activeReady<2*activeStock && turn<101;
  demandReport[turn]={activeStock,activeReady,activeMask,fullBanks,int(scheduled&&!partial)};'''.replace('   CGFloat_UNUSED\n',''))
rep('  // Untasked workers build local brace frontiers without disrupting a paired hold.\n  if(turn<bankStaffTurn+32){\n   vector<tuple<int,int,int,int>> jobs;\n   for(int p:bankWalls)if(!wall[p]){','  // Untasked workers build eligible brace frontiers without disrupting a paired hold.\n  if(turn<bankStaffTurn+32){\n   vector<tuple<int,int,int,int>> jobs;\n   for(int p:bankWalls)if(activeWalls[p]&&!wall[p]){')
rep(' if(const char* p=getenv("AHC008_ARTICLE_LOG")){',''' if(const char* p=getenv("AHC008_DEMAND_LOG")){
  ofstream f(p);
  for(int t=0;t<300;t++)if(g.demandReport[t][0]){
   const auto& q=g.demandReport[t];
   f<<t<<' '<<q[0]<<' '<<q[1]<<' '<<q[2]<<' '<<q[3]<<' '<<q[4]<<'\\n';
  }
 }
 if(const char* p=getenv("AHC008_ARTICLE_LOG")){''')
(root/'integrated.cpp').write_text(s)
