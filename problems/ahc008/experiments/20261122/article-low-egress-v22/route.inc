 struct BraceStop{int stance,tube;array<int,2> work;};
 array<vector<BraceStop>,10> braceRoute;
 array<int,10> braceNext{},braceEgress{};
 bool braceInit=false;
 int braceNominal=-1;
 array<array<int,5>,10> braceReport{};
 array<int,10> braceInitialJobs{};

 void initBraceRoutes(){
  braceInit=true;braceEgress.fill(-1);
  const int m=humans.size();
  if(m>6||bankN!=2)return;
  bitset<V> closed;for(int p:bankWalls)closed[p]=true;
  array<int,V> noBody;noBody.fill(-1);
  vector<BraceStop> atomic;
  vector<vector<int>> blocks;
  for(int bank=0;bank<2;bank++)for(int k=0;k<7;k++){
   vector<int> block;
   const int tube=13*bank+2*k;
   const auto& x=banks[tube];const int r=x.r,a=bankShift+7*bank;
   for(int j=0;j<4;j++){
    block.push_back(atomic.size());
    atomic.push_back({bp(r,a+2+j),tube,{bp(r-1,a+2+j),bp(r+1,a+2+j)}});
   }
   blocks.push_back(move(block));
  }
  // For 13 tube rows, 7 paired sweeps partition the 14 brace rows exactly.
  // All stance-to-stance paths are in the final shoulder-free graph.
  const int k=atomic.size(), states=1<<m, INF=9999;
  vector<BankPath> initial(m),path(k);
  for(int i=0;i<m;i++)initial[i].run(humans[i],closed,noBody);
  for(int s=0;s<k;s++)path[s].run(atomic[s].stance,closed,noBody);
  int best=INF;
  for(int mode=0;mode<8;mode++){
   vector<int> ord;
   for(int i=0;i<14;i++){
    int b=i/7,j=i%7;
    if(mode&1)b^=1;
    if((mode>>1&1)^(i/7&1))j=6-j;
    auto blk=blocks[b*7+j];
    if((i&1)^bool(mode&4))reverse(blk.begin(),blk.end());
    ord.insert(ord.end(),blk.begin(),blk.end());
   }
   vector<int> chain(k);
   bool viable=true;
   for(int i=1;i<k;i++){
    int d=path[ord[i-1]].d[atomic[ord[i]].stance];
    if(d<0){viable=false;break;}
    chain[i]=chain[i-1]+d;
   }
   if(!viable)continue;
   vector<vector<pair<int,int>>> exit(m,vector<pair<int,int>>(k,{INF,-1}));
   for(int h=0;h<m;h++)for(int j=0;j<k;j++){
    const auto& x=banks[atomic[ord[j]].tube];
    for(int side=0;side<2;side++){
     int here=side?x.h:x.l,other=side?x.l:x.h;
     int d=path[ord[j]].d[here],peer=INF;
     for(int i=0;i<m;i++)if(i!=h&&initial[i].d[other]>=0)peer=min<int>(peer,initial[i].d[other]);
     if(d<0||peer==INF)continue;
     // Distinct opposing accessibility is a tie, not a new guard assignment.
     auto score=pair(64*d+min(63,peer),here);
     exit[h][j]=min(exit[h][j],score);
    }
   }
   vector<vector<int>> dp(states,vector<int>(k+1,INF));
   struct Pre{int last=-1,worker=-1,start=-1;};
   vector<vector<Pre>> pre(states,vector<Pre>(k+1));
   dp[0][0]=0;
   for(int mask=0;mask<states;mask++)for(int l=0;l<k;l++)if(dp[mask][l]<best){
    for(int h=0;h<m;h++)if(!(mask>>h&1)){
     int approach=initial[h].d[atomic[ord[l]].stance];
     if(approach<0)continue;
     int nm=mask|(1<<h);
     for(int r=l;r<k;r++){
      if(approach+2*(r-l+1)+chain[r]-chain[l]>=best)break;
      auto [e,dst]=exit[h][r];
      if(dst<0)continue;
      int cost=max(dp[mask][l],approach+2*(r-l+1)+chain[r]-chain[l]+e/64);
      if(cost<dp[nm][r+1])dp[nm][r+1]=cost,pre[nm][r+1]={mask,h,l};
     }
    }
   }
   for(int mask=0;mask<states;mask++)if(dp[mask][k]<best){
    best=dp[mask][k];
    for(auto& row:braceRoute)row.clear();
    braceEgress.fill(-1);
    for(int j=k,mask2=mask;j>0;){
     auto [last,h,l]=pre[mask2][j];
     assert(h>=0);
     for(int s=l;s<j;s++)braceRoute[h].push_back(atomic[ord[s]]);
     braceEgress[h]=exit[h][j-1].second;
     mask2=last;j=l;
    }
   }
  }
  if(best<INF){
   braceNominal=best;
   for(int i=0;i<m;i++)braceInitialJobs[i]=int(braceRoute[i].size())*2;
  }
 }

 void runBraceRoutes(bitset<V>& usedPeople,bitset<V>& build,bitset<V>& moveTo){
  if(!braceInit)initBraceRoutes();
  // Beyond the fixed nominal makespan use the existing free-worker repair,
  // rather than assigning lasting private roles.
  if(braceNominal<0||turn>=braceNominal+8)return;
  bitset<V> closed=build;
  for(int p=0;p<V;p++)if(wall[p])closed[p]=true;
  for(int p:bankWalls)closed[p]=true;
  array<int,V> body;body.fill(-1);
  for(int k=0;k<int(banks.size());k++){
   auto& x=banks[k];
   if(all_of(x.side.begin(),x.side.end(),[&](int p){return wall[p];}))
    for(int p:x.inner)body[p]=k;
  }
  for(int i=0;i<int(humans.size());i++)if(!usedPeople[i]){
   auto& route=braceRoute[i]; int& current=braceNext[i],from=humans[i];
   auto &report=braceReport[i];
   if(current==int(route.size())&&braceEgress[i]<0)continue;
   BankPath path;path.run(from,closed,body);
   const auto moveToward=[&](int st){
    int q=path.step[st];
    if(q<0||build[q])return false;
    act[i]=MOVE[dir(from,q)];usedPeople[i]=true;moveTo[q]=true;++report[0];
    return true;
   };
   while(current<int(route.size())){
    auto [st,k,work]=route[current];
    bool pending=false;
    for(int p:work)if(!wall[p]&&!build[p]&&!block[p]&&!moveTo[p])pending=true;
    if(!pending){
     for(int p:work)if(!wall[p]&&!build[p])++report[2];
     ++current;continue;} // repairs remain ordinary jobs
    if(st!=from){
     if(moveToward(st))break;
     for(int p:work)if(!wall[p]&&!build[p])++report[2];
     ++current;continue;
    }
    for(int p:work)if(!wall[p]&&!build[p]&&!block[p]&&!moveTo[p]){
     assert(dir(from,p)>=0);
     act[i]=BUILD[dir(from,p)];build[p]=true;usedPeople[i]=true;++report[1];
     break;
    }
    if(usedPeople[i])break;
    ++current;
   }
   if(!usedPeople[i]&&current==int(route.size())&&braceEgress[i]>=0){
    int e=braceEgress[i];
    if(from==e){report[3]=turn+1;braceEgress[i]=-1;}
    else if(!moveToward(e))braceEgress[i]=-1;
    else ++report[4]; // never hold a berth
   }
  }
 }
