#pragma GCC optimize("O3,unroll-loops")
#include <bits/stdc++.h>
using namespace std;
constexpr int D=32,Z=D*D;
constexpr int MAXWORK = 2800, STALL = 14, PLANCOUNT = 1, PREDEPOCH = 7;
constexpr array<int,4> W{-D,D,-1,1};
constexpr char go[]="UDLR", build[]="udlr";
struct Room{
 vector<int> inside, shell, gate;
 vector<pair<int,int>> work;
 bool done=false; int ready=-1,start=-1,last=-1,remaining=9999;
};
struct Game{
 vector<int> hp,pp,pt;
 bool wall[Z]{},blocked[Z],put[Z],into[Z],reservation[Z];
 int step[10][Z],dist[10][Z];
 vector<Room> rooms;
 void read(){
  int n,m; cin>>n; while(n--){int x,y,t;cin>>x>>y>>t;pp.push_back(x*D+y);pt.push_back(t);}
  cin>>m;while(m--){int x,y;cin>>x>>y;hp.push_back(x*D+y);}
  for(int q=0;q<Z;q++){int x=q/D,y=q%D;wall[q]=(min(x,y)==0 || max(x,y)==31);}
 }
 int pred[Z], wd[Z], open[Z];
 void prep(){
  memset(pred,0,sizeof pred); memset(wd,0,sizeof wd);memset(open,0,sizeof open);
  for(int i=0;i<(int)pp.size();i++){
   int p=pp[i]; if(dist[0][p]<0)continue;
   const int rad=pt[i]==1?PREDEPOCH-2:pt[i]==2?PREDEPOCH-1:PREDEPOCH;
   short ds[Z];fill(ds,ds+Z,-1);int queue[Z],h=0,t=0;
   queue[t++]=p;ds[p]=0;int sum=0;
   auto weight=[&](int d){return 1+(rad+1)*(rad+1)-d*d;};
   while(h<t){int a=queue[h++],d=ds[a];sum+=weight(d);if(d==rad)continue;
    for(int z:W)if(ds[a+z]<0&&!wall[a+z]){ds[a+z]=d+1;queue[t++]=a+z;}
   }
   for(int j=0;j<t;j++)pred[queue[j]]+=weight(ds[queue[j]])*(pt[i]==4?95:150)/sum;

  }
  for(int x=1;x<=31;x++)for(int y=1;y<=31;y++){
   int q=x*D+y;
   int h=100;
   for(int i=0;i<(int)hp.size();i++)if(dist[i][q]>=0)h=min(h,dist[i][q]);
   open[q]=(!wall[q]&&dist[0][q]>=0);
   wd[q]=wall[q]||dist[0][q]<0?0:(65+min(h,20)*3+35*blocked[q]);
   for(auto *arr:{pred,wd,open})arr[q]+=arr[q-1]+arr[q-D]-arr[q-D-1];
  }
 }
 int range(const int *arr,int l,int r,int a,int b){
  return arr[r*D+b]-(l>0?arr[(l-1)*D+b]:0)-(a>0?arr[r*D+a-1]:0)+(l>0&&a>0?arr[(l-1)*D+a-1]:0);
 }
 void choose(int turn){
  prep();
  double best=-1;array<int,4> shape{};
  static const array<int,10> dims{2,3,4,5,6,8,10,12,16,30};
  for(int h:dims)for(int w:dims){
   if(h*w<6||h*w>380)continue;
   for(int l=1;l+h<=31;l++)for(int a=1;a+w<=31;a++){
    int r=l+h-1,b=a+w-1;
    int ar=range(open,l,r,a,b);if(ar<4||ar>330)continue;
    int work=range(wd,l-1,l-1,a,b)+range(wd,r+1,r+1,a,b)+range(wd,l,r,a-1,a-1)+range(wd,l,r,b+1,b+1);
    if(work<65||work>MAXWORK)continue;
    int pts=range(pred,l,r,a,b);
    if(pts<25)continue;
    int preserve=0;
    for(auto &p:hp)preserve+=l<=p/D&&p/D<=r&&a<=p%D&&p%D<=b;
    int duplicate=0;
    for(auto &j:rooms)if(!j.done && !j.shell.empty() && ranges::any_of(j.shell,[&](int p){return l-1<=p/D&&p/D<=r+1&&a-1<=p%D&&p%D<=b+1;}))duplicate++;
    // Effective construction effort. Large pockets are only preferred when they offer distinctly more catch.
    
    double v=(pts-1.00*ar)/(440.+work*.50+ar*.15+70*preserve+6000*duplicate);
    if(v>best)best=v,shape={l,r,a,b};
   }
  }
  if(best<=0)return;
  auto [l,r,a,b]=shape;
  Room room;room.start=room.last=turn;
  for(int x=l;x<=r;x++)for(int y=a;y<=b;y++)if(!wall[x*D+y])room.inside.push_back(x*D+y);
  for(int x=l-1;x<=r+1;x++)for(int y=a-1;y<=b+1;y++){
   bool edgeX=x==l-1||x==r+1,edgeY=y==a-1||y==b+1;
   if(edgeX==edgeY)continue;
   int q=x*D+y;
   if(!wall[q]&&dist[0][q]>=0)room.shell.push_back(q);
  }
  int ga=-1,gc=INT_MAX;
  for(int q:room.shell){
   bool reserved=false;
   for(auto &ro:rooms)if(!ro.done&&(ranges::find(ro.shell,q)!=ro.shell.end()||ranges::find(ro.gate,q)!=ro.gate.end()))reserved=true;
   if(reserved)continue;
   for(int d:W){int s=q+d;
    if(wall[s]||enclosed(room,s)||ranges::find(room.shell,s)!=room.shell.end())continue;
    int cost=INT_MAX;
    for(int i=0;i<(int)hp.size();i++)if(dist[i][s]>=0)cost=min(cost,dist[i][s]);
    if(cost==INT_MAX)continue;
    int degree=0,inside=0; for(int z:W)if(!wall[q+z])degree++,inside+=enclosed(room,q+z);
    if(!inside)continue;
    cost+=2*blocked[q]+2*(4-degree);
    if(cost<gc)gc=cost,ga=q;
   }
  }
  if(ga<0)return;
  room.gate.push_back(ga);erase(room.shell,ga);
  for(int q:room.shell)for(auto d:W)if(!wall[q+d] && !enclosed(room,q+d) && ranges::find(room.shell,q+d)==room.shell.end() && ranges::find(room.gate,q+d)==room.gate.end())room.work.push_back({q,q+d});
  rooms.push_back(move(room));
 }
 void bfs(int i){
  auto *ds=dist[i],*ss=step[i]; fill(ds,ds+Z,-1);int que[Z],h=0,t=0;
  que[t++]=hp[i];ds[hp[i]]=0;ss[hp[i]]=-1;
  while(h<t){int a=que[h++];for(int d=0;d<4;d++){int b=a+W[d];if(ds[b]<0&&!wall[b]&&!put[b]){ds[b]=ds[a]+1;ss[b]=h==1?d:ss[a];que[t++]=b;}}}
 }
 int getd(int a,int b){return find(W.begin(),W.end(),b-a)-W.begin();}
 bool enclosed(const Room&r,int a){return ranges::find(r.inside,a)!=r.inside.end();}
 void plan(int turn){
  int m=hp.size();
  memset(blocked,0,sizeof blocked); memset(put,0,sizeof put);memset(into,0,sizeof into); memset(reservation,0,sizeof reservation);
  for(int p:pp){blocked[p]=true;for(int w:W)blocked[p+w]=true;}
  for(int p:hp)blocked[p]=true;
  for(int i=0;i<m;i++)bfs(i);
  for(auto &r:rooms)if(!r.done){
   int left=0;for(auto q:r.shell)left+=!wall[q];
   if(r.remaining>left)r.remaining=left,r.last=turn; 
   if(left && turn-r.last>STALL)r.done=true;
   if(r.ready>=0 && turn-r.ready>43)r.done=true;
  }
  for(auto &r:rooms)if(!r.done && turn-r.start>58)r.done=true;
  int pending=0;for(auto &r:rooms)pending+=!r.done && r.ready<0;
  if(pending<PLANCOUNT && (turn<290) && turn%3==0)choose(turn);
  vector<int> np(rooms.size()),nh(rooms.size()),left(rooms.size()),ng(rooms.size()),target(m,-1);
  string acts(m,'.');
  for(int j=0;j<(int)rooms.size();j++){
   auto &r=rooms[j]; if(r.done)continue;
   for(int p:pp)np[j]+=enclosed(r,p);
   for(int p:hp)nh[j]+=enclosed(r,p) || ranges::find(r.gate,p)!=r.gate.end();
   for(int q:r.shell)left[j]+=!wall[q];
   for(int q:r.gate)ng[j]+=!wall[q];
   if(left[j]||!ng[j])continue;
   if(r.ready<0)r.ready=turn;
   if(!np[j]||nh[j]||(np[j]==1&&r.inside.size()>95&&turn-r.ready<8&&turn<275))continue;
   bool good=true;vector<pair<int,int>> workers;
   for(int q:r.gate){if(wall[q])continue;
    if(blocked[q]||into[q]){good=false;break;}
    int worker=-1;
    for(int i=0;i<m;i++)if(acts[i]=='.' && target[i]<0 && getd(hp[i],q)<4 && ranges::none_of(workers,[&](auto s){return s.first==i;})){worker=i;break;}
    if(worker<0){good=false;break;}
    workers.emplace_back(worker,q);
   }
   if(good){
    for(auto [i,q]:workers){acts[i]=build[getd(hp[i],q)];put[q]=true;}
    r.done=true;
   }
  }
  for(int repeat=0;repeat<m;repeat++){
   double best=-1e9;int bi=-1,bq=-1,bs=-1;
   for(int i=0;i<m;i++)if(acts[i]=='.'&&target[i]<0){
    for(int j=0;j<(int)rooms.size();j++){
     auto &r=rooms[j];if(r.done)continue;
     auto trywork=[&](int q,int s,double bonus){
      if(wall[q]||put[q]||reservation[q]||wall[s]||put[s]||dist[i][s]<0)return;
      int d=dist[i][s];
      if(d==0&&(blocked[q]||into[q]))return;
      if(d>0&&put[hp[i]+W[step[i][s]]])return;
      double v=bonus-d-2.0*(blocked[q]);
      if(v>best){best=v;bi=i;bq=q;bs=s;}
     };
     if(left[j]){
      double bonus=2-.15*left[j]+min(3,np[j])*1.5;
      for(auto [q,s]:r.work)trywork(q,s,bonus);
      // Also accessible interior cells can complete stalled side partitions.
      if(left[j]<=3)for(int q:r.shell)if(!wall[q])for(auto w:W)if(enclosed(r,q+w))trywork(q,q+w,bonus-4);
     }else if(np[j])for(int q:r.gate)for(int w:W)if(!enclosed(r,q+w))trywork(q,q+w,2.0*np[j]+1.5);
     if(!left[j]&&nh[j]&&(enclosed(r,hp[i])||ranges::find(r.gate,hp[i])!=r.gate.end())){
      for(int q:r.gate)for(int w:W)if(!enclosed(r,q+w)&&!wall[q+w]){
       int s=q+w,d=dist[i][s];
       if(d>0&&!put[hp[i]+W[step[i][s]]]&&6-d>best)best=6-d,bi=i,bq=q,bs=s;
      }
     }
    }
   }
   if(bi<0)break;
   target[bi]=bq;reservation[bq]=true;
   if(bs==hp[bi]){
    // Gates are only ever built by the coordinated closure stage.
    bool g=false;for(auto &r:rooms)if(!r.done)g|=ranges::find(r.gate,bq)!=r.gate.end();
    if(!g){acts[bi]=build[getd(bs,bq)];put[bq]=true;}
   }else{
    int d=step[bi][bs];acts[bi]=go[d];into[hp[bi]+W[d]]=true;
   }
  }
  cout<<acts<<endl;
  for(int i=0;i<m;i++){
   if(acts[i]=='.')continue;
   if(islower(acts[i])) wall[hp[i]+W[find(build,build+4,acts[i])-build]]=true;
   else hp[i]+=W[find(go,go+4,acts[i])-go];
  }
 }
};
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 Game g;g.read();
 for(int t=0;t<300;t++){
  g.plan(t);
  for(int &p:g.pp){string s;if(!(cin>>s))return 0;for(char a:s)if(a!='.')p+=W[find(go,go+4,a)-go];}
 }
}
