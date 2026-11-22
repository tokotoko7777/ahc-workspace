 // Untasked workers build incident tube shells, without reserving guard jobs.
 if(turn<bankStaffTurn+32){
  array<int,V> benefit{};
  array<vector<int>,V> incident;
  bitset<V> unavailable=build,shoulders;
  array<int,V> body;body.fill(-1);
  for(int p=0;p<V;p++)if(wall[p])unavailable[p]=true;
  for(int p:bankWalls)shoulders[p]=true;
  for(int k=0;k<n;k++){
   const auto& x=banks[k];
   for(int p:x.inner)body[p]=k;
   if(wall[x.g]||wall[x.k]||build[x.g]||build[x.k]||
      any_of(x.inner.begin(),x.inner.end(),[&](int p){return wall[p]||build[p];}))continue;
   int missing=0;for(int p:x.side)missing+=!unavailable[p];
   if(!missing)continue;
   for(int p:x.side)if(!unavailable[p]){
    benefit[p]+=bankBraceCredit(missing);
    incident[p].push_back(k);
   }
  }
  // Conservative independent partner routes: no ready or unready body
  // shortcuts, and none of the nominal brace cells can be traversed.
  array<BankPath,10> exterior;
  for(int i=0;i<m;i++)if(!usedPeople[i])exterior[i].run(humans[i],unavailable|shoulders,body);
  vector<array<array<int,2>,10>> partner(n);
  for(int k=0;k<n;k++)for(int i=0;i<m;i++){
   const auto& x=banks[k];auto& q=partner[k][i];q={99,99};
   for(int j=0;j<m;j++)if(j!=i&&!usedPeople[j]){
    if(int d=exterior[j].d[x.l];d>=0)q[0]=min(q[0],d);
    if(int d=exterior[j].d[x.h];d>=0)q[1]=min(q[1],d);
   }
  }
  vector<tuple<int,int,int,int,int>> jobs;
  for(int p:bankWalls)if(!wall[p]){
   for(int i=0;i<m;i++)if(!usedPeople[i])for(int st:nb[p])if(st>=0&&!wall[st]&&!build[st]){
    int d=workers[i].bfs.distance(st);
    if(d<0||(d==0&&(block[p]||moveTo[p])))continue;
    int support=0;
    if(d>0&&!shoulders[st]&&body[st]<0){
     for(int k:incident[p]){
      auto& x=banks[k];int dl=md(st,x.l),dr=md(st,x.h);
      support=max({support,bankPartnerTie(dl,partner[k][i][1]),bankPartnerTie(dr,partner[k][i][0])});
     }
    }
    jobs.emplace_back(20+benefit[p]-6*d-(block[p]?15:0),support,i,p,st);
   }
  }
  sort(jobs.rbegin(),jobs.rend());bitset<V> used;
  for(auto [pri,support,i,p,st]:jobs)if(!usedPeople[i]&&!used[p]){
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
