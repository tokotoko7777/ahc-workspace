 struct BankJob{int tube,a,b,eta;double value;};
 struct BankPath {
  array<short,V> d,step;
  void run(int src,const bitset<V>&closed,const array<int,V>&body){
   d.fill(-1);step.fill(-1);d[src]=0;
   int q[V],len=1;q[0]=src;
   for(int i=0;i<len;i++)for(int v:nb[q[i]])if(v>=0&&d[v]<0&&!closed[v]&&(body[v]<0||body[v]==body[src])){
    d[v]=d[q[i]]+1;step[v]=(i==0?v:step[q[i]]);q[len++]=v;
   }
  }
 };
 array<int,10> previousTube{}, previousDoor{};
 bool priorPlan=false;
 array<int,300> integratedMode{},integratedReady{},integratedLocks{};
 // TERRY-style weighted set packing, exact over the tiny worker mask.
 vector<BankJob> packBankJobs(const vector<vector<BankJob>>& jobs,int cap)const{
  const int n=1<<humans.size(), z=jobs.size();
  vector<double> dp(n,-1),next;
  vector<vector<pair<short,short>>> back(z,vector<pair<short,short>>(n,{-1,-1}));
  dp[0]=0;
  for(int k=0;k<z;k++){
   next=dp;
   for(int u=0;u<n;u++)if(dp[u]>=0&&popcount(unsigned(u))<2*cap){
    for(int j=0;j<int(jobs[k].size());j++){
     const auto &x=jobs[k][j];int t=(1<<x.a)|(1<<x.b);
     if(!(t&u)&&dp[u]+x.value>next[t|u]){
      next[t|u]=dp[u]+x.value;back[k][t|u]={short(u),short(j)};
     }
    }
   }
   dp.swap(next);
  }
  int u=max_element(dp.begin(),dp.end())-dp.begin();
  vector<BankJob> out;
  for(int k=z-1;k>=0&&u;k--){
   auto [ot,j]=back[k][u];
   if(j>=0){out.push_back(jobs[k][j]);u=ot;}
  }
  return out;
 }
 static int comp(int a,const array<int,V>& group){return group[a];}
 string integratedBankPlan(){
  prep();
  const int m=humans.size(),n=banks.size();
  int rem=0;bitset<V> hum;bitset<V> build,moveTo,usedPeople;
  array<bool,V> service{};
  for(int h:humans){hum[h]=1;service[group[h]]=true;}
  for(Pet p:pets)rem+=service[group[p.p]];
  if(turn==160)residual160=rem;
#if __has_include("last_capture.hpp")
  if(turn>=180&&rem>0&&rem<=2&&!overBudget()){
   vector<lastcap::Pet> alive;
   for(auto const& p:pets)alive.push_back({p.p,p.t});
   lastcap::Board roads{},retreat{};
   int principal=group[humans[0]];
   for(int c=0;c<=bankN;c++)for(int r=0;r<S;r++){
    int p=bp(r,bankShift+7*c);
    if(group[p]==principal)roads[p]=1;
   }
   for(int r:{0,29})for(int c=bankShift;c<=bankShift+7*bankN;c++)
    if(group[bp(r,c)]==principal)roads[bp(r,c)]=1;
   // Relaxation protects the actual common safe anchor (the current crew
   // root), not empty nominal roads. FinalCert still requires crew routes.
   retreat[humans[0]]=1;
   if(auto q=finalCapture.plan(turn,wall,humans,alive,roads,retreat)){
    integratedMode[turn]=finalCapture.protectionTier==1?3:5;
    return *q;
   }
  }
#endif
  if(turn>=235){bankRelease=235;integratedMode[turn]=4;dogPhase=9;return plan();}
  vector<int> ready(n),enabled(n),count(n);
  array<int,4> local{};int total=0;
  for(int k=0;k<n;k++){
   const auto &x=banks[k];
   ready[k]=all_of(x.side.begin(),x.side.end(),[&](int p){return wall[p];});
   local[x.b]+=ready[k];total+=ready[k];
   if(!ready[k]||wall[x.g]||wall[x.k])continue;
   int h=0;for(int q:x.inner)h+=hum[q];
   for(Pet p:pets)count[k]+=find(x.inner.begin(),x.inner.end(),p.p)!=x.inner.end();
   enabled[k]=!h;
  }
  integratedReady[turn]=total;
  // Exact immediate closures as a distinct, unconditional optimization.
  vector<vector<BankJob>> immediate(n),jobs(n);
  for(int k=0;k<n;k++)if(enabled[k]&&count[k]&&!block[banks[k].g]&&!block[banks[k].k]){
   const auto& x=banks[k];
   for(int a=0;a<m;a++)if(humans[a]==x.l)for(int b=0;b<m;b++)if(humans[b]==x.h)
    immediate[k].push_back({k,a,b,0,double(count[k])});
  }
  if(any_of(immediate.begin(),immediate.end(),[](const auto& x){return !x.empty();})){
   for(const auto& j:packBankJobs(immediate,m/2)){
    const auto& x=banks[j.tube];
    act[j.a]=BUILD[dir(x.l,x.g)];act[j.b]=BUILD[dir(x.h,x.k)];
    build[x.g]=build[x.k]=true;usedPeople[j.a]=usedPeople[j.b]=true;
    integratedLocks[turn]+=count[j.tube];
   }
  }
  const bool scheduled=turn>=35&&*max_element(local.begin(),local.end())>=5;
  const bool partial=3*total<2*n && turn<101;
  array<BankPath,10> paths;
  if(scheduled){
   integratedMode[turn]=partial?1:2;
   bitset<V> unavailable=build;
   array<int,V> body;body.fill(-1);
   for(int q=0;q<V;q++)unavailable[q]=unavailable[q]||wall[q];
   for(int k=0;k<n;k++)if(ready[k])for(int p:banks[k].inner)body[p]=k;
   for(int i=0;i<m;i++)if(!usedPeople[i])paths[i].run(humans[i],unavailable,body);
   struct Pair{int a,b,d;};
   vector<vector<Pair>> pairInfo(n);
   int maxEta=0;
   for(int k=0;k<n;k++)if(ready[k]&&!wall[banks[k].g]&&!wall[banks[k].k]&&!build[banks[k].g]){
    const auto& x=banks[k];
    for(int a=0;a<m;a++)if(!usedPeople[a]&&paths[a].d[x.l]>=0&&paths[a].d[x.l]<=Horizon)
     for(int b=0;b<m;b++)if(a!=b&&!usedPeople[b]&&paths[b].d[x.h]>=0&&paths[b].d[x.h]<=Horizon){
      const int d=max(paths[a].d[x.l],paths[b].d[x.h]);
      // t+0 was handled by exact checking, so this pair can hold for t+1.
      pairInfo[k].push_back({a,b,max(1,d)});maxEta=max(maxEta,max(1,d));
     }
   }
   if(maxEta&&!overBudget()){
    if(!postReady)observedMotionGraph();
    vector<vector<Forecast>> placeholder; // no per-pet forecast retained
    vector<array<double,Horizon+1>> noBad(n),noPets(n),mass(n);
    for(int k=0;k<n;k++){noBad[k].fill(1);noPets[k].fill(1);mass[k].fill(0);}
    array<vector<int>,40> gateBad;
    for(int k=0;k<n;k++)if(!pairInfo[k].empty()){
     const auto& x=banks[k];auto& b=gateBad[k];
     b={x.g,x.k};
     for(int g:{x.g,x.k})for(int p:nb[g])if(p>=0&&!wall[p])b.push_back(p);
     sort(b.begin(),b.end());b.erase(unique(b.begin(),b.end()),b.end());
    }
    for(int pi=0;pi<int(pets.size());pi++)if(service[group[pets[pi].p]]){
     Forecast f{};forecastPet(pi,maxEta,f);
     for(int k=0;k<n;k++)if(!pairInfo[k].empty()){
      const auto& x=banks[k];
      for(int h=1;h<=maxEta;h++){
       double best=f[h][x.inner[1]]+f[h][x.inner[2]],bad=0;
       for(int p:gateBad[k])bad+=f[h][p];
       bad=clamp(bad,0.,1.);best=clamp(best,0.,1.-bad);
       // Update E[legal and number captured] without unstable division.
       mass[k][h]=mass[k][h]*(1.-bad)+noBad[k][h]*best;
       noBad[k][h]*=1.-bad;
       noPets[k][h]*=1.-bad-best;
      }
     }
    }
    for(int k=0;k<n;k++){
     array<BankJob,1024> best{};for(auto& j:best)j.value=-1;
     const auto &x=banks[k];
     for(auto [a,b,d]:pairInfo[k]){
      double joint=mass[k][d],occupied=max(0.,noBad[k][d]-noPets[k][d]);
      if(occupied<.025)continue;
      double score=joint/(1.+d);
      if(priorPlan&&previousTube[a]==k&&previousDoor[a]==x.l&&previousTube[b]==k&&previousDoor[b]==x.h)score*=1.10;
      int mask=(1<<a)|(1<<b);
      if(score>best[mask].value)best[mask]={k,a,b,d,score};
     }
     for(auto [a,b,d]:pairInfo[k]){const int mask=(1<<a)|(1<<b);auto& j=best[mask];
      if(j.value>=0){jobs[k].push_back(j);j.value=-1;}}
    }
   }
   previousTube.fill(-1);previousDoor.fill(-1);priorPlan=true;
   if(any_of(jobs.begin(),jobs.end(),[](const auto& x){return !x.empty();})){
    for(auto j:packBankJobs(jobs,partial?1:m/2)){
     const auto& x=banks[j.tube];
     for(auto [i,goal]:{pair{j.a,x.l},pair{j.b,x.h}}){
      usedPeople[i]=true;previousTube[i]=j.tube;previousDoor[i]=goal;
      int s=paths[i].step[goal];
      if(s>=0&&!build[s])act[i]=MOVE[dir(humans[i],s)],moveTo[s]=true;
     }
    }
   }
  }
  // Untasked workers build local brace frontiers without disrupting a paired hold.
  if(turn<bankStaffTurn+32){
   vector<tuple<int,int,int,int>> jobs;
   for(int p:bankWalls)if(!wall[p]){
    int progress=0,axis=bankRot?p/S:p%S,r=bankRot?p%S:p/S,start=axis-((axis-bankShift-2)%7);
    for(int k=0;k<4;k++)if(start+k>=0&&start+k<S)progress+=wall[bp(r,start+k)];
    for(int i=0;i<m;i++)if(!usedPeople[i])for(int st:nb[p])if(st>=0&&!wall[st]&&!build[st]){
     int d=workers[i].bfs.distance(st);
     if(d<0||(d==0&&(block[p]||moveTo[p])))continue;
     jobs.emplace_back(20+2*progress-6*d-(block[p]?15:0),i,p,st);
    }
   }
   sort(jobs.rbegin(),jobs.rend());bitset<V> used;
   for(auto [pri,i,p,st]:jobs)if(!usedPeople[i]&&!used[p]){
    int h=humans[i];
    if(st==h){
     if(block[p]||build[p]||moveTo[p])continue;
     act[i]=BUILD[dir(h,p)];build[p]=true;
    }else{
     int q=workers[i].bfs.first_step(st);
     if(q<0||build[q])continue;
     act[i]=MOVE[dir(h,q)];moveTo[q]=true;
    }
    used[p]=true;usedPeople[i]=true;
   }
  }
  if(scheduled&&!partial)parkUnassigned(ready,paths,usedPeople,build,moveTo,service);
  return act;
 }
 void parkUnassigned(const vector<int>& ready,const array<BankPath,10>& paths,bitset<V> usedPeople,
      const bitset<V>& build,bitset<V>& moveTo,const array<bool,V>& service){
  struct Park{int goal=-1,tube=-1;double value=0.;};
  int m=humans.size(),n=pets.size(),states=1<<m;
  vector<array<Park,10>> dest(n);
  for(int pi=0;pi<n;pi++)if(service[group[pets[pi].p]]){
   const auto &p=pets[pi];
   for(int k=0;k<int(banks.size());k++)if(ready[k]&&!wall[banks[k].g]&&!wall[banks[k].k]&&!build[banks[k].g]){
    const auto &x=banks[k];
    int pd=min(md(p.p,x.inner[1]),md(p.p,x.inner[2]));
    if(pd>8)continue;
    for(int side=0;side<2;side++){
     int goal=side?x.h:x.l,other=side?x.l:x.h,partner=24;
     for(int i=0;i<m;i++)if(md(humans[i],goal)>0)partner=min(partner,md(humans[i],other));
     for(int i=0;i<m;i++)if(!usedPeople[i]&&paths[i].d[goal]>=0){
      int d=paths[i].d[goal];
      double value=1./(1.+.28*d+.38*pd+.12*partner);
      if(dest[pi][i].value<value)dest[pi][i]={goal,k,value};
     }
    }
   }
  }
  vector<double> dp(states,-1),next;
  vector<vector<int>> prev(n,vector<int>(states,-1));
  dp[0]=0.;
  for(int j=0;j<n;j++){
   next=dp;
   for(int s=0;s<states;s++)if(dp[s]>=0.)
    for(int i=0;i<m;i++)if(!(s>>i&1)&&dest[j][i].goal>=0){
     int z=s|(1<<i);double value=dp[s]+dest[j][i].value;
     if(value>next[z]){next[z]=value;prev[j][z]=i;}
    }
   dp.swap(next);
  }
  int s=max_element(dp.begin(),dp.end())-dp.begin();
  bitset<V> taken;
  for(int j=n-1;j>=0&&s;j--)if(int i=prev[j][s];i>=0){
   s^=1<<i;int goal=dest[j][i].goal;
   if(taken[goal])continue;
   taken[goal]=true;usedPeople[i]=true;
   int q=paths[i].step[goal];
   if(q>=0&&!build[q])act[i]=MOVE[dir(humans[i],q)],moveTo[q]=1;
  }
  // Geometric standby only when there is no distinct reachable pet job.
  // A lane is represented by its closest still-open, prepared mid-row stance.
  for(int i=0;i<m;i++)if(!usedPeople[i]){
   int best=1000,goal=-1;
   for(int k=0;k<int(banks.size());k++)if(ready[k]&&!wall[banks[k].g]&&!wall[banks[k].k]&&!build[banks[k].g]){
    const auto &x=banks[k];
    for(int v:{x.l,x.h})if(!taken[v]&&paths[i].d[v]>=0){
     int value=paths[i].d[v]+abs(x.r-14)/2;
     for(int j=0;j<m;j++)if(j!=i&&(taken[humans[j]]||usedPeople[j]))
      value+=max(0,4-md(v,humans[j]));
     if(value<best)best=value,goal=v;
    }
   }
   if(goal<0)continue;
   taken[goal]=true;
   int q=paths[i].step[goal];
   if(q>=0&&!build[q])act[i]=MOVE[dir(humans[i],q)],moveTo[q]=1;
  }
 }
