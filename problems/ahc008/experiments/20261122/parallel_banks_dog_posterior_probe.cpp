#pragma GCC optimize("O3")
#include <bits/stdc++.h>
#include "dog-target-filter.hpp"
using namespace std;
constexpr int V=900,S=30;
constexpr char DIR[]="UDLR";
array<array<int,4>,V> near4;
struct Frame {vector<int> p,h;string wall;vector<string> commands;};
struct Tube {int g,k,l,r;array<int,4> floor;vector<int> side;};
struct Graph {
 int m;const Frame& f;array<array<short,V>,10> d;
 explicit Graph(const Frame& x):m(x.h.size()),f(x){
  for(int w=0;w<m;w++){
   auto &a=d[w];a.fill(-1);int Q[V],n=1;Q[0]=f.h[w];a[Q[0]]=0;
   for(int j=0;j<n;j++)for(int v:near4[Q[j]])if(v>=0&&f.wall[v]=='0'&&a[v]<0)Q[n++]=v,a[v]=a[Q[j]]+1;
  }
 }
 vector<int> next(int x)const{
  vector<int> q;for(int y:near4[x])if(y>=0&&f.wall[y]=='0')q.push_back(y);return q;
 }
 template<class Dest> void first(int x,int target,double weight,Dest fn)const{
  vector<int> choices;for(int j=0;j<m;j++)if(d[j][x]>0)choices.push_back(j);
  if(choices.empty()){
   auto adj=next(x);
   for(int q:adj)fn(q,0,weight/adj.size(),false);
   return;
  }
  if(target>0&&d[target-1][x]>0)choices={target-1};
  auto adj=next(x);
  for(int j:choices){
   int n=0;for(int q:adj)n+=d[j][q]==d[j][x]-1;
   for(int q:adj)if(d[j][q]==d[j][x]-1)
    fn(q,j+1,weight/(choices.size()*n),true);
  }
 }
};
using Belief=array<double,11>;
double tilt(int from,int to,int prev){
 return to-from==prev?1.6:to-from==-prev?.5:1.;
}
double first_prob(const Graph& g,int x,int real,const Belief& b,int heading){
 double ok=0,total=0;
 for(int j=0;j<=g.m;j++)if(b[j]>0)g.first(x,j,b[j],[&](int q,int,double w,bool directed){
  if(!directed)return;
  w*=tilt(x,q,heading);total+=w;
  if(q==real)ok+=w;
 });
 if(total<=0)throw runtime_error("no directed step probability");
 return ok/total;
}
array<double,2> hitting(const Graph& g,int x,const Tube& tube,const Belief& b,int heading){
 unordered_map<int,double> dp,nxt;
 for(int j=0;j<=g.m;j++)if(b[j]>0)dp[x*11+j]=b[j];
 double done=0;array<double,2> ans{};
 for(int t=1;t<=3;t++){
  nxt.clear();double absorbed=0,denom=0;
  for(auto [key,pr]:dp){
   int p=key/11,target=key%11;
   g.first(p,target,pr,[&](int q,int actual,double w,bool directed){
    if(t==1)w*=tilt(p,q,heading);
    denom+=w;
    auto add=[&](int r,int a,double prob){
     if(r==tube.floor[1]||r==tube.floor[2])absorbed+=prob;
     else nxt[r*11+a]+=prob;
    };
    if(!directed){add(q,0,w);return;}
    auto steps=g.next(q);
    for(int r:steps){
     int a=(g.d[actual-1][q]==0||g.d[actual-1][r]==0)?0:actual;
     add(r,a,w/steps.size());
    }
   });
  }
  if(t==1){
   for(auto &[k,p]:nxt)p/=denom;
   absorbed/=denom;
  }
  done+=absorbed;
  if(t==1)ans[0]=done;
  dp.swap(nxt);
 }
 ans[1]=done;
 assert(ans[1]>=ans[0]&&ans[1]<1.00000000001);
 return ans;
}
bool present(int q,const vector<int>& z){return find(z.begin(),z.end(),q)!=z.end();}
bool intact(const Frame& f,const Tube& x){
 if(f.wall[x.g]=='1'||f.wall[x.k]=='1')return false;
 for(int q:x.side)if(f.wall[q]!='1')return false;
 for(int q:x.floor)if(present(q,f.h)||f.wall[q]=='1')return false;
 return true;
}
bool legal(const Frame& f,const Tube& x,int id){
 if(!intact(f,x)||!count(x.floor.begin(),x.floor.end(),f.p[id]))return false;
 for(int z:f.p){
  if(z==x.g||z==x.k)return false;
  for(int v:near4[z])if(v==x.g||v==x.k)return false;
 }
 return !present(x.g,f.h)&&!present(x.k,f.h);
}
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 for(int p=0;p<V;p++)near4[p]={p>=S?p-S:-1,p+S<V?p+S:-1,p%S?p-1:-1,p%S<S-1?p+1:-1};
 int n,m,b,release;cin>>n>>m>>b>>release;
 vector<int> type(n);for(int &x:type)cin>>x;
 vector<Tube> tubes(b);
 for(auto &x:tubes){
  int k;cin>>x.l>>x.g>>x.k>>x.r;for(int &p:x.floor)cin>>p;
  cin>>k;x.side.resize(k);for(int &p:x.side)cin>>p;
 }
 vector<Frame> frames(300);
 for(auto &f:frames){
  f.p.resize(n);f.h.resize(m);f.commands.resize(n);
  for(int &x:f.p)cin>>x;for(int &x:f.h)cin>>x;cin>>f.wall;
  for(auto &v:f.commands)cin>>v;
  if(f.wall.size()!=V)throw runtime_error("invalid frame");
 }
 vector<AHC008DogTargetFilter> posterior(n,AHC008DogTargetFilter(m));
 vector<int> last(n);
 Belief reset{};reset[0]=1.;cout<<setprecision(12);
 for(int t=0;t<300;t++){
  const auto& f=frames[t];Graph graph(f);
  for(int i=0;i<n;i++)if(type[i]==4){
   Belief b{};b[0]=posterior[i].reset_probability();
   for(int j=0;j<m;j++)b[j+1]=posterior[i].probability(j);
   int p=f.p[i],cur=p;
   vector<int> seen;
   for(char c:f.commands[i]){
    auto d=find(DIR,DIR+4,c)-DIR;if(d>=4)throw runtime_error("invalid pet command");
    cur=near4[cur][d];if(cur<0||f.wall[cur]=='1')throw runtime_error("invalid pet state");
    seen.push_back(cur);
   }
   if(seen.size()==2){
    double a=first_prob(graph,p,seen[0],b,0),u=first_prob(graph,p,seen[0],reset,0),h=first_prob(graph,p,seen[0],reset,last[i]);
    if(min({a,u,h})<=0)throw runtime_error("impossible observation");
    cout<<"D "<<t<<" "<<i<<" "<<a<<" "<<u<<" "<<h<<"\n";
   }
   if(t>=35&&t+3<release)for(int j=0;j<int(tubes.size());j++){
    auto const& x=tubes[j];
    if(p!=x.l&&p!=x.r&&p!=x.g&&p!=x.k&&!count(x.floor.begin(),x.floor.end(),p))continue;
    if(!intact(f,x)||legal(f,x,i))continue;
    int eta=999;
    for(int u=0;u<m;u++)for(int v=0;v<m;v++)if(u!=v&&graph.d[u][x.l]>=0&&graph.d[v][x.r]>=0)
     eta=min<int>(eta,max(graph.d[u][x.l],graph.d[v][x.r]));
    if(eta>2)continue;
    const auto post=hitting(graph,p,x,b,0),uni=hitting(graph,p,x,reset,0),head=hitting(graph,p,x,reset,last[i]);
    cout<<"P "<<t<<" "<<i<<" "<<j<<" "<<eta<<" "<<post[0]<<" "<<uni[0]<<" "<<head[0]<<" "
        <<post[1]<<" "<<uni[1]<<" "<<head[1]<<"\n";
   }
   auto next=graph.next(p);
   if(!posterior[i].update(p,next,seen,[&](int w,int q){return graph.d[w][q];}))
    throw runtime_error("posterior failed on official move");
   last[i]=seen.size()==2?seen[0]-p:0;
  }
 }
}
