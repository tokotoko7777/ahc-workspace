#pragma once
#include <algorithm>
#include <array>
#include <bit>
#include <cstdlib>
#include <limits>
#include <optional>
#include <queue>
#include <string>
#include <tuple>
#include <vector>
namespace lastcap {
constexpr int S=30,V=900,INF=100000000;
using Board=std::array<unsigned char,V>;
struct Pet{int p,t;};
inline std::array<int,4> neighbors(int p){return {p>=S?p-S:-1,p+S<V?p+S:-1,p%S?p-1:-1,p%S<S-1?p+1:-1};}
inline int dist1(int a,int b){return std::abs(a/S-b/S)+std::abs(a%S-b%S);}
inline int direction(int a,int b){auto z=neighbors(a);return int(std::find(z.begin(),z.end(),b)-z.begin());}
inline constexpr char MOVE[]="UDLR",BUILD[]="udlr";
struct BFS{
 std::array<int,V> d,first;
 int run(int src,const Board& can){
  d.fill(-1);first.fill(-1);if(!can[src])return 0;
  std::array<int,V> q;int n=1;q[0]=src;d[src]=0;
  for(int i=0;i<n;++i)for(int x:neighbors(q[i]))if(x>=0&&can[x]&&d[x]<0){
   d[x]=d[q[i]]+1;first[x]=i?first[q[i]]:x;q[n++]=x;
  }
  return n;
 }
};
struct Flow{
 struct Edge{int to,rev,cap;};
 std::vector<std::vector<Edge>> g;
 std::vector<int> d,at;
 explicit Flow(int n):g(n),d(n),at(n){}
 void add(int u,int v,int c){g[u].push_back({v,int(g[v].size()),c});g[v].push_back({u,int(g[u].size())-1,0});}
 void levels(int s){
  std::fill(d.begin(),d.end(),-1);std::vector<int> q{s};d[s]=0;
  for(size_t i=0;i<q.size();++i)for(auto e:g[q[i]])if(e.cap&&d[e.to]<0){d[e.to]=d[q[i]]+1;q.push_back(e.to);}
 }
 int send(int u,int t,int n){
  if(u==t)return n;
  for(int &j=at[u];j<(int)g[u].size();++j){
   auto &e=g[u][j];
   if(e.cap&&d[e.to]==d[u]+1){
    if(int k=send(e.to,t,std::min(n,e.cap))){e.cap-=k;g[e.to][e.rev].cap+=k;return k;}
   }
  }
  return 0;
 }
 int maxflow(int s,int t){
  int v=0;
  while(v<INF){levels(s);if(d[t]<0)break;std::fill(at.begin(),at.end(),0);
   while(v<INF){int f=send(s,t,INF-v);if(!f)break;v+=f;}}
  return v;
 }
};
struct Config{
 int start=146, maxAttempt=48, maxTotal=96, maxWalls=30, minArea=185;
 int retry=5, finalWait=9, maxPet=2;
};
struct Planner{
 Config cfg{};
 struct Shape{std::vector<int> cut;Board can{};int kept=0,trapped=0,cost=0;double value=-1;};
 Shape active;
 int began=-1, deadline=-1, total=0, retryAt=0, finalSince=-1;
 int attempts=0, actions=0, placed=0, closes=0, escapes=0, lanePlans=0, retreatPlans=0, protectionTier=0;
 std::array<int,10> pref{};
 Planner(){pref.fill(-1);}
 void reset(int turn){active={};began=-1;finalSince=-1;retryAt=turn+cfg.retry;pref.fill(-1);}
 static Board walkable(const Board& wall){Board b{};for(int p=0;p<V;++p)b[p]=!wall[p];return b;}
 static Board occupied(const std::vector<int>& humans,const std::vector<Pet>& pets){
  Board b{};
  for(auto a:pets){b[a.p]=1;for(int q:neighbors(a.p))if(q>=0)b[q]=1;}
  for(int h:humans)b[h]=1;
  return b;
 }
 static bool certify(Shape& z,const Board& wall,const std::vector<int>& humans,const std::vector<Pet>& pets,const Board& root){
  z.can=walkable(wall);for(int p:z.cut)z.can[p]=0;
  BFS b;z.kept=b.run(humans[0],z.can);
  for(int h:humans)if(b.d[h]<0)return false;
  for(int p=0;p<V;++p)if(b.d[p]<0)z.can[p]=0;
  z.trapped=0;
  for(auto a:pets)if(root[a.p]){
   if(std::find(z.cut.begin(),z.cut.end(),a.p)!=z.cut.end())return false;
   z.trapped+=!z.can[a.p];
  }
  if(!z.trapped)return false;
  for(int p:z.cut)if(!wall[p]){
   bool stance=false;for(int q:neighbors(p))if(q>=0&&z.can[q])stance=true;
   if(!stance)return false;
  }
  return true;
 }
 Shape search(int turn,const Board& wall,const Board& main,int size,
              const std::vector<int>& humans,const std::vector<Pet>& pets,const Board& protect){
  auto blocked=occupied(humans,pets);
  std::vector<BFS> ds(humans.size());
  auto open=walkable(wall);
  for(size_t i=0;i<humans.size();++i)ds[i].run(humans[i],open);
  std::array<int,V> eta;eta.fill(90);
  for(int p=0;p<V;++p)if(main[p])for(int q:neighbors(p))if(q>=0&&main[q])for(auto &b:ds)
    if(b.d[q]>=0)eta[p]=std::min(eta[p],b.d[q]);
  std::vector<int> ap;
  for(auto a:pets)if(main[a.p])ap.push_back(a.p);
  Shape best;
  const int n=ap.size(),H=humans.size(),horizon=std::min({299-turn,cfg.maxAttempt,cfg.maxTotal-total});
  if(horizon<2)return best;
  Board protectedNow=protect;
  for(int h:humans)protectedNow[h]=1;
  for(int subset=1;subset<(1<<n);++subset)for(int core:{1,2,3})for(int loss:{1,4}){
   if(n==2&&subset==3&&dist1(ap[0],ap[1])>15)continue;
   std::array<int,V> dist;dist.fill(100);
   for(int p=0;p<V;++p)for(int j=0;j<n;++j)if(subset>>j&1)
    dist[p]=std::min(dist[p],dist1(p,ap[j]));
   const int R=core+5;
   constexpr int s=V*2,t=V*2+1;
   Flow f(t+1);
   bool possible=true;
   for(int p=0;p<V;++p)if(main[p]&&dist[p]<R){
    if(dist[p]<=core&&protectedNow[p])possible=false;
    bool border=false;
    for(int q:neighbors(p))if(q>=0&&main[q]&&dist[q]>=R)border=true;
    const int cost=(protectedNow[p]||dist[p]<=core||eta[p]+2>=horizon)?INF:31+eta[p]*3+blocked[p]*6;
    f.add(p*2,p*2+1,cost);
    f.add(s,p*2,loss);
    if(border||protectedNow[p])f.add(s,p*2+1,INF),f.add(s,p*2,INF);
    if(dist[p]<=core)f.add(p*2,t,INF),f.add(p*2+1,t,INF);
    for(int q:neighbors(p))if(q>=0&&main[q]&&dist[q]<R)f.add(p*2+1,q*2,INF);
   }
   if(!possible||f.maxflow(s,t)>=INF)continue;
   Shape z;
   for(int p=0;p<V;++p)if(main[p]&&dist[p]<R&&f.d[p*2]>=0&&f.d[p*2+1]<0)z.cut.push_back(p);
   if(z.cut.empty()||int(z.cut.size())>cfg.maxWalls||!certify(z,wall,humans,pets,main))continue;
   int required=std::min(cfg.minArea,size*7/10);
   if(z.kept<required||z.kept*100<size*(z.trapped==1?65:45))continue;
   int far=0,near=0,left=0;
   for(int p:z.cut){far=std::max(far,eta[p]);near+=eta[p];left+=blocked[p];}
   int dur=far+2+int((z.cut.size()*2+H-1)/H)+left/2;
   if(dur>=horizon)continue;
   z.cost=dur;
   z.value=(double(z.kept*(1<<z.trapped))-size)/(1.+.045*dur+.034*z.cut.size()+.012*near);
   if(z.value>best.value)best=std::move(z);
  }
  return best;
 }
 std::optional<std::string> plan(int turn,const Board& wall,const std::vector<int>& humans,
                                 const std::vector<Pet>& pets,const Board& protect={},const Board& retreat={}){
  if(humans.empty()||humans.size()>10||turn<cfg.start||turn>=300)return std::nullopt;
  auto open=walkable(wall);
  BFS rootB;int size=rootB.run(humans[0],open);
  for(int h:humans)if(rootB.d[h]<0){if(began>=0)reset(turn);return std::nullopt;}
  Board root{};
  for(int p=0;p<V;++p)root[p]=rootB.d[p]>=0;
  int reachable=0;for(auto a:pets)reachable+=root[a.p];
  if(!reachable||reachable>cfg.maxPet){if(began>=0)reset(turn);return std::nullopt;}
  if(began>=0&&(turn>=deadline||total>=cfg.maxTotal||!certify(active,wall,humans,pets,root))){
   if(turn<deadline)++escapes;
   reset(turn);
  }
  if(began<0){
   if(turn<retryAt||total>=cfg.maxTotal)return std::nullopt;
   active=search(turn,wall,root,size,humans,pets,protect);
   protectionTier=1;
   if(active.value<0&&protect!=retreat){
    active=search(turn,wall,root,size,humans,pets,retreat);
    protectionTier=2;
   }
   if(active.value<0){reset(turn);return std::nullopt;}
   ++(protectionTier==1?lanePlans:retreatPlans);
   began=turn;deadline=std::min({300,turn+cfg.maxAttempt,turn+cfg.maxTotal-total});++attempts;
  }
  auto blocked=occupied(humans,pets);
  std::vector<int> todo;
  for(int p:active.cut)if(!wall[p])todo.push_back(p);
  if(todo.empty()){reset(turn);return std::nullopt;}
  const int M=humans.size(),K=todo.size();
  std::vector<BFS> nav(M);
  for(int i=0;i<M;++i)nav[i].run(humans[i],active.can);
  struct Job{int stand=-1,d=INF;};
  std::vector<std::vector<Job>> job(M,std::vector<Job>(K));
  for(int j=0;j<K;++j)for(int i=0;i<M;++i)for(int q:neighbors(todo[j])){
   if(q<0||nav[i].d[q]<0)continue;
   if(nav[i].d[q]<job[i][j].d)job[i][j]={q,nav[i].d[q]};
  }
  std::string out(M,'.');
  if(K<=M){
   // Match all remaining doors to distinct builders with an exact bit DP.
   // Primary goal = exact fastest completion. Secondary travel and churn are tie heuristics.
   std::vector<int> dp(1<<M,INF),choice(1<<M,-1);
   dp[0]=0;
   for(int bits=0;bits<(1<<M);++bits){
    int j=std::popcount(unsigned(bits));
    if(j>=K||dp[bits]>=INF)continue;
    for(int i=0;i<M;++i)if(!(bits>>i&1)&&job[i][j].stand>=0){
     int d=job[i][j].d;
     int v=std::max(dp[bits]/10000,d)*10000+dp[bits]%10000+11*d+(pref[i]!=todo[j]);
     int next=bits|1<<i;
     if(v<dp[next]){dp[next]=v;choice[next]=i;}
    }
   }
   int best=0;
   for(int b=1;b<(1<<M);++b)if(std::popcount(unsigned(b))==K&&dp[b]<INF&&(!best||dp[b]<dp[best]))best=b;
   if(best){
    std::vector<int> who(K);
    for(int j=K-1,b=best;j>=0;--j){who[j]=choice[b];b^=1<<who[j];}
    bool ready=true,legal=true;
    for(int j=0;j<K;++j){ready&=job[who[j]][j].d==0;legal&=!blocked[todo[j]];}
    if(ready&&legal){
     for(int j=0;j<K;++j){int i=who[j];out[i]=BUILD[direction(humans[i],todo[j])];}
     placed+=K;++closes;
     // Next observation independently verifies all walls and the pet split.
    }else{
     if(ready){if(finalSince<0)finalSince=turn;}
     else finalSince=-1;
     if(finalSince>=0&&turn-finalSince>=cfg.finalWait){reset(turn);return std::nullopt;}
     for(int j=0;j<K;++j){
      int i=who[j],q=job[i][j].stand;pref[i]=todo[j];
      if(job[i][j].d>0)out[i]=MOVE[direction(humans[i],nav[i].first[q])];
     }
    }
    ++total;++actions;
    return out;
   }
  }
  // Prep: dispatch whole crew from the known final exterior. A path never
  // touches a prospective wall or the eventual animal component.
  std::vector<std::tuple<int,int,int>> order;
  for(int j=0;j<K;++j)for(int i=0;i<M;++i){
   int d=job[i][j].d;if(d>=INF||(!d&&blocked[todo[j]]))continue;
   int score=(d==0?100:0)-8*d-15*blocked[todo[j]]+3*(pref[i]==todo[j]);
   order.emplace_back(score,i,j);
  }
  std::sort(order.rbegin(),order.rend());
  std::vector<bool> taken(K),assigned(M);
  for(auto [score,i,j]:order)if(!assigned[i]&&!taken[j]){
   int q=job[i][j].stand;pref[i]=todo[j];
   if(job[i][j].d==0){out[i]=BUILD[direction(humans[i],todo[j])];++placed;}
   else out[i]=MOVE[direction(humans[i],nav[i].first[q])];
   assigned[i]=taken[j]=true;
  }
  if(out.find_first_not_of('.')==std::string::npos){reset(turn);return std::nullopt;}
  ++total;++actions;
  return out;
 }
};
}
