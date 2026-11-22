 // Search actual two-cell separators, not perturbed weights on a distance ball.
 Shape offsetFrontier(int turn,const Board& wall,const Board& main,int size,
     const std::vector<int>& humans,const std::vector<Pet>& pets,const Board& protect) const {
  Shape best;
  if(humans.size()!=5||turn<235||turn>296||size<305)return best;
  const auto blocked=occupied(humans,pets);
  std::vector<int> target;
  for(const auto a:pets)if(main[a.p])target.push_back(a.p);
  if(target.empty()||target.size()>2)return best;
  const auto open=walkable(wall);
  std::array<BFS,5> approx;
  for(int i=0;i<5;i++)approx[i].run(humans[i],open);
  Board want{};
  std::vector<int> candidates;
  for(int p=0;p<V;++p)if(main[p]&&!protect[p]&&!blocked[p]){
   bool near=false, shoulder=false, staff=false;
   for(int q:target)near|=dist1(p,q)<=8;
   if(!near)continue;
   for(int q:neighbors(p)){
    if(q<0||wall[q])shoulder=true;
    if(q>=0)for(auto& b:approx)staff|=b.d[q]>=0&&b.d[q]<=3;
   }
   if(shoulder&&staff){
    // Both mouths are necessary; avoid dressing a singleton bottleneck
    // with a redundant second planned wall.
    Board one=main;one[p]=0;
    BFS test;test.run(humans.front(),one);
    bool reallyTwo=true;
    for(int q:target)reallyTwo&=test.d[q]>=0;
    if(reallyTwo)candidates.push_back(p),want[p]=1;
   }
  }
  if(candidates.size()<2)return best;
  Board live{};for(int p:target)++live[p];
  std::array<int,V> in,low,sub,animals;
  // Different row AND column excludes ordinary straight two-cell seam.
  for(int a:candidates){
   in.fill(-1);int clock=0;
   std::vector<int> mouths;
   auto dfs=[&](auto&& self,int p,int parent)->void{
    in[p]=low[p]=clock++;sub[p]=1;animals[p]=live[p];
    for(int q:neighbors(p))if(q>=0&&q!=a&&main[q]&&q!=parent){
     if(in[q]>=0){low[p]=std::min(low[p],in[q]);continue;}
     self(self,q,p);sub[p]+=sub[q];animals[p]+=animals[q];
     low[p]=std::min(low[p],low[q]);
     if(p>a&&want[p]&&p/30!=a/30&&p%30!=a%30&&low[q]>=in[p]&&animals[q]&&sub[q]>=4&&sub[q]<=48)
      mouths.push_back(p);
    }
   };
   dfs(dfs,humans.front(),-1);
   std::sort(mouths.begin(),mouths.end());
   mouths.erase(std::unique(mouths.begin(),mouths.end()),mouths.end());
   for(int b:mouths){
    Shape now;now.cut={a,b};
    if(!certify(now,wall,humans,pets,main)||now.kept<300||100*now.kept<80*size)continue;
    Board island=main;
    for(int p=0;p<V;++p)if(now.can[p])island[p]=0;
    island[a]=island[b]=0;
    bool compact=true;
    for(int p:target)if(island[p]){
     BFS z;int cells=z.run(p,island);
     if(cells<4||cells>48){compact=false;break;}
    }
    if(!compact)continue;
    std::array<std::array<int,2>,5> d;
    for(int i=0;i<5;i++){
     d[i]={99,99};BFS nav;nav.run(humans[i],now.can);
     for(int j=0;j<2;j++)for(int q:neighbors(now.cut[j]))if(q>=0&&nav.d[q]>=0)
      d[i][j]=std::min(d[i][j],nav.d[q]);
    }
    int eta=99,moves=99;
    for(int i=0;i<5;i++)for(int j=0;j<5;j++)if(i!=j){
     int e=std::max(d[i][0],d[j][1]),m=d[i][0]+d[j][1];
     if(std::pair(e,m)<std::pair(eta,moves))eta=e,moves=m;
    }
    if(eta>3||turn+eta+1>=300)continue;
    now.cost=eta+1;
    now.value=1000000.*now.trapped-5000.*eta+now.kept-.01*moves;
    if(now.value>best.value)best=std::move(now);
   }
  }
  return best;
 }
