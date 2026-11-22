 vector<CatTargetFilter<V>> catPosterior;
 ShortestFirstMasks<V> catBfs;
 clock_t catObserveTicks=0,catForecastTicks=0;
 struct CatNLL{int turn,id,dir;double old,next;};
 vector<CatNLL> catNll;
 struct CatProbe{int turn,tube,eta,id,a,b;double oldSafe,newSafe,others;};
 vector<CatProbe> catProbe;
 array<float,4> headingFirst(int id,int q)const{
  array<float,4> p{};
  int d=catHead[id],n=degree[q];
  if(!n)return p;
  bool use=d>=0&&nb[q][d]>=0&&!wall[nb[q][d]];
  for(int i=0;i<4;i++)if(nb[q][i]>=0&&!wall[nb[q][i]])p[i]=(use?.5f:1.f)/n;
  if(use)p[d]+=.5f;
  return p;
 }
 void auditCatObserved(int id,int src,const string& moves){
  if(turn>=225||pets[id].t!=5)return;
  clock_t start=clock();
  int d=int(find(MOVE,MOVE+4,moves[0])-MOVE);
  int q=(d<4?nb[src][d]:src),r=q;
  if(moves.size()>1&&moves[1]!='.')r=nb[q][find(MOVE,MOVE+4,moves[1])-MOVE];
  if(d<4&&(integratedMode[turn]==1||integratedMode[turn]==2)){
   auto p=headingFirst(id,src);
   auto b=catPosterior[id].next(src,nb,[&](int x){return !wall[x];},catBfs);
   catNll.push_back({turn,id,d,p[d],b[d]});
  }
  catPosterior[id].observe(src,d,q,r,nb,[&](int x){return !wall[x];},catBfs);
  catObserveTicks+=clock()-start;
 }
 void auditCatForecast(const vector<array<int,3>>& best,
                      const array<bool,V>& service){
  int horizon=0;
  for(auto x:best)if(x[0]<=3)horizon=max(horizon,x[0]);
  if(!horizon)return;
  bool have=false;
  for(auto p:pets)have|=p.t==5&&service[group[p.p]];
  if(!have)return;
  clock_t start=clock();
  vector<Forecast> old(pets.size()),diff(pets.size());
  for(int i=0;i<(int)pets.size();i++)if(service[group[pets[i].p]]){
   forecastPet(i,horizon,old[i]);
   auto [src,type]=pets[i];
   if(type!=5)continue;
   auto d=catPosterior[i].next(src,nb,[&](int x){return !wall[x];},catBfs);
   Prob a{};
   if(!degree[src])a[src]=1.f;
   else for(int k=0;k<4;k++)
    if(nb[src][k]>=0&&!wall[nb[src][k]])a[nb[src][k]]=d[k];
   uniformStep(a,diff[i][1],src,1);
   for(int h=2;h<=horizon;h++){
    a.fill(0);
    uniformStep(diff[i][h-1],a,src,2*(h-1));
    uniformStep(a,diff[i][h],src,2*h-1);
   }
  }
  int n=pets.size();
  for(int k=0;k<(int)best.size();k++){
   const auto [h,a,b]=best[k];
   if(h>3)continue;
   const auto& x=banks[k];
   if(any_of(x.inner.begin(),x.inner.end(),[&](int p){return wall[p];}))continue;
   vector<int> forbidden{x.g,x.k};
   for(int g:{x.g,x.k})for(int q:nb[g])if(q>=0&&!wall[q])forbidden.push_back(q);
   sort(forbidden.begin(),forbidden.end());forbidden.erase(unique(forbidden.begin(),forbidden.end()),forbidden.end());
   vector<double> legal(n,1.);
   for(int j=0;j<n;j++)if(service[group[pets[j].p]]){
    double q=0.;
    for(int p:forbidden)q+=old[j][h][p];
    legal[j]=1.-clamp(q,0.,1.);
   }
   for(int j=0;j<n;j++)if(pets[j].t==5&&service[group[pets[j].p]]){
    int p=x.inner[1],q=x.inner[2];
    double baseline=old[j][h][p]+old[j][h][q];
    if(baseline==0.)continue; // identical fixed-board geometric support can favor both
    double posterior=diff[j][h][p]+diff[j][h][q];
    double other=1.;
    for(int i=0;i<n;i++)if(i!=j)other*=legal[i];
    catProbe.push_back({turn,k,h,j,a,b,baseline,posterior,other});
   }
  }
  catForecastTicks+=clock()-start;
 }
