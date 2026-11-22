 vector<AHC008DogTargetFilter> dogBelief;
 vector<int> catHead;
 array<array<short,V>,10> postDist{};
 bool postReady=false;
 static constexpr int Horizon=10,DogModelHorizon=3;
 static constexpr double eps=1e-7;
 using Prob=array<float,V>;
 using Forecast=array<Prob,Horizon+1>;
 array<array<int,4>,V> openEdge{};
 array<int,V> degree;
 void observedMotionGraph(){
  for(int p=0;p<V;p++){
   degree[p]=0;
   if(wall[p])continue;
   for(int q:nb[p])if(q>=0&&!wall[q])openEdge[p][degree[p]++]=q;
  }
  for(int i=0;i<int(humans.size());i++){
   auto &d=postDist[i];d.fill(-1);
   int q[V],n=1;q[0]=humans[i];d[q[0]]=0;
   for(int j=0;j<n;j++)for(int k=0;k<degree[q[j]];k++){
    int b=openEdge[q[j]][k];
    if(d[b]<0)d[b]=d[q[j]]+1,q[n++]=b;
   }
  }
  postReady=true;
 }
 void setupObservedMotion(){
  for(Pet p:pets)dogBelief.emplace_back(humans.size());
  catHead.assign(pets.size(),-1);
 }
 void ingestMotion(int i,int start,const string& reply){
  int type=pets[i].t;
  if(type==5){catHead[i]=reply[0]=='.'?-1:int(find(MOVE,MOVE+4,reply[0])-MOVE);return;}
  if(type!=4 || !degree[start])return;
  int seen[2],n=0,p=start;
  for(char c:reply)if(c!='.'&&n<2){p=nb[p][find(MOVE,MOVE+4,c)-MOVE];seen[n++]=p;}
  if(n&&!dogBelief[i].update(start,span(openEdge[start].data(),degree[start]),span(seen,n),
       [&](int a,int q){return postDist[a][q];})){
   dogBelief[i]=AHC008DogTargetFilter(humans.size());
  }
 }
 void uniformStep(const Prob& from,Prob& to,int src,int reach)const{
  int r=src/S,c=src%S;
  for(int x=max(0,r-reach);x<=min(S-1,r+reach);x++){
   int len=reach-abs(x-r);
   for(int y=max(0,c-len);y<=min(S-1,c+len);y++){
    int p=pos(x,y);float f=from[p];
    if(f==0.)continue;
    int n=degree[p];
    if(!n){to[p]+=f;continue;}
    f/=n;
    for(int k=0;k<n;k++)to[openEdge[p][k]]+=f;
   }
  }
 }
 void dogStep(array<Prob,11>& cur,array<Prob,11>& next,int src,int t)const{
  int reach=2*(t-1), resetIndex=humans.size(),cx=src/S,cy=src%S;
  for(int x=max(0,cx-reach);x<=min(S-1,cx+reach);x++){
   int len=reach-abs(x-cx);
   for(int y=max(0,cy-len);y<=min(S-1,cy+len);y++){
    int p=pos(x,y),nd=degree[p],cnt=0;
    float reset=cur[resetIndex][p];
    array<int,10> valid;
    for(int j=0;j<resetIndex;j++)if(postDist[j][p]>0)valid[cnt++]=j;else reset+=cur[j][p];
    if(!cnt){
     if(reset<=eps)continue;
     if(nd==0)next[resetIndex][p]+=reset;
     else for(int k=0;k<nd;k++)next[resetIndex][openEdge[p][k]]+=reset/nd;
     continue;
    }
    float each=reset/cnt;
    for(int j=0;j<cnt;j++){
     int who=valid[j];float mass=cur[who][p]+each;
     if(mass<=eps)continue;
     int d=postDist[who][p],nn=0;array<int,4> choices;
     for(int k=0;k<nd;k++){int q=openEdge[p][k];if(postDist[who][q]==d-1)choices[nn++]=q;}
     if(!nn)continue;
     mass/=nn;
     for(int k=0;k<nn;k++){
      int q=choices[k];
      if(!degree[q])next[postDist[who][q]==0?resetIndex:who][q]+=mass;
      else for(int l=0;l<degree[q];l++){
       int v=openEdge[q][l];
       next[postDist[who][q]==0||postDist[who][v]==0?resetIndex:who][v]+=mass/degree[q];
      }
     }
    }
   }
  }
 }
 void forecastPet(int i,int horizon,Forecast& f)const{
  auto [src,type]=pets[i];f[0][src]=1;
  int done=0;
  if(type==4&&postReady){
   array<Prob,11> state{};
   const int m=humans.size();
   for(int j=0;j<m;j++)state[j][src]=dogBelief[i].probability(j);
   state[m][src]=dogBelief[i].reset_probability();
   for(int h=1;h<=min(horizon,DogModelHorizon);h++){
    array<Prob,11> nxt{};dogStep(state,nxt,src,h);state.swap(nxt);
    for(int j=0;j<=m;j++)for(int p=0;p<V;p++)f[h][p]+=state[j][p];
    done=h;
   }
  }
  int speed=min(type,3);if(type>=4)speed=2;
  for(int h=done+1;h<=horizon;h++){
   Prob a{},b{};
   if(type==5&&h==1&&catHead[i]>=0){
    int q=nb[src][catHead[i]];
    if(q>=0&&!wall[q]){
     a[q]+=.5f;
     for(int k=0;k<degree[src];k++)a[openEdge[src][k]]+=.5f/degree[src];
    }else uniformStep(f[0],a,src,0);
   }else uniformStep(f[h-1],a,src,(h-1)*speed);
   for(int s=1;s<speed;s++){
    b.fill(0);uniformStep(a,b,src,(h-1)*speed+s);a.swap(b);
   }
   f[h]=a;
  }
 }
