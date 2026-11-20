#include <bits/stdc++.h>
#include "../../library/rooted-terminal-tree.hpp"
#include "../../library/dsu.hpp"
using namespace std;

#ifndef AHC020_STEINER
#define AHC020_STEINER 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int MAXN=100;
using Int=long long;
constexpr Int INF=1LL<<58;
struct Edge {int a,b,w;};
struct Instance {
  int n,m,k;
  vector<pair<int,int>> station,client;
  vector<Edge> edges;
  vector<vector<int>> radius,available;
  vector<vector<pair<int,int>>> reaches;
  vector<int> mst_parent,mst_edge,mst_order,activation;
  vector<int> edge_order;
  array<array<Int,MAXN>,MAXN> dist{};
  array<array<int,MAXN>,MAXN> next{};
  void initialize() {
    cin>>n>>m>>k;
    station.resize(n);client.resize(k);edges.resize(m);
    for(auto& [x,y]:station)cin>>x>>y;
    for(auto& [a,b,w]:edges){cin>>a>>b>>w;--a;--b;}
    for(auto& [x,y]:client)cin>>x>>y;
    activation.assign(n,INT_MAX);
    for(const auto& e:edges){
      activation[e.a]=min(activation[e.a],e.w);
      activation[e.b]=min(activation[e.b],e.w);
    }
    radius.assign(n,vector<int>(k,5001));
    reaches.resize(n);
    available.resize(k);
    for(int i=0;i<n;++i)for(int j=0;j<k;++j){
      Int x=station[i].first-client[j].first,y=station[i].second-client[j].second;
      Int d=x*x+y*y;
      if(d>25000000)continue;
      int r=sqrt(double(d));
      if(Int(r)*r<d)++r;
      radius[i][j]=r;
      available[j].push_back(i);
      reaches[i].push_back({r,j});
    }
    for(auto& v:reaches)sort(v.begin(),v.end());
    edge_order.resize(m);
    iota(edge_order.begin(),edge_order.end(),0);
    sort(edge_order.begin(),edge_order.end(),[&](int a,int b){
      return pair{edges[a].w,a}<pair{edges[b].w,b};
    });
    vector<vector<pair<int,int>>> tree(n);
    Dsu dsu(n);
    for(int e:edge_order){
      auto [a,b,w]=edges[e];
      if(dsu.unite(a,b))tree[a].push_back({b,e}),tree[b].push_back({a,e});
    }
    mst_parent.assign(n,-1);mst_edge.assign(n,-1);mst_order={0};mst_parent[0]=0;
    for(int i=0;i<int(mst_order.size());++i){
      int v=mst_order[i];
      for(auto [u,e]:tree[v])if(mst_parent[u]<0){
        mst_parent[u]=v;mst_edge[u]=e;mst_order.push_back(u);
      }
    }
    assert(int(mst_order.size())==n);
  }
  Int mst(const vector<int>& power,vector<unsigned char>& scratch,vector<int>* chosen=nullptr) const {
    rooted_terminal_tree(mst_order,mst_parent,[&](int v){return power[v]>0;},scratch);
    Int cost=0;
    if(chosen)chosen->assign(m,0);
    for(int i=1;i<n;++i)if(scratch[i]){
      int e=mst_edge[i];cost+=edges[e].w;
      if(chosen)(*chosen)[e]=1;
    }
    return cost;
  }
  void shortest_paths() {
    for(int i=0;i<n;++i){
      dist[i].fill(INF);next[i].fill(-1);dist[i][i]=0;
    }
    for(int e=0;e<m;++e){
      auto [a,b,w]=edges[e];
      if(w<dist[a][b]) dist[a][b]=dist[b][a]=w,next[a][b]=next[b][a]=e;
    }
    for(int v=0;v<n;++v)for(int a=0;a<n;++a)for(int b=0;b<n;++b)
      if(dist[a][v]+dist[v][b]<dist[a][b]){
        dist[a][b]=dist[a][v]+dist[v][b];next[a][b]=next[a][v];
      }
  }
};

struct Radio {
  const Instance& in;
  vector<int> power,end,covered;
  Int cost=0;
  Radio(const Instance& in):in(in),power(in.n),end(in.n),covered(in.k) {}
  void set(int v,int p) {
    if(power[v]==p)return;
    cost+=Int(p)*p-Int(power[v])*power[v];
    power[v]=p;
    const auto& order=in.reaches[v];
    int i=upper_bound(order.begin(),order.end(),pair{p,in.k})-order.begin();
    while(end[v]<i)++covered[order[end[v]++].second];
    while(end[v]>i)--covered[order[--end[v]].second];
  }
};
vector<int> find_power(const Instance& in,Clock::time_point stop) {
  Radio s(in);
  vector<int> nearest(in.n);
  for(int j=0;j<in.k;++j){
    const auto& cand=in.available[j];
    assert(!cand.empty());
    int v=*min_element(cand.begin(),cand.end(),[&](int a,int b){
      return in.radius[a][j]<in.radius[b][j];
    });
    nearest[v]=max(nearest[v],in.radius[v][j]);
  }
  for(int i=0;i<in.n;++i)s.set(i,nearest[i]);
  vector<unsigned char> tree;
  Int current=in.mst(s.power,tree)+s.cost,best=current;
  vector<int> answer=s.power;
  mt19937 rng(204323);
  vector<pair<int,int>> undo;
  vector<int> lost;
  array<bool,MAXN> used{};
  constexpr int iters=12000;
  double temp=1e6;
  for(int it=0;it<iters;++it){
    if((it&31)==0){
      if(Clock::now()>=stop)break;
      temp=1e6*pow(0.004,it/double(iters));
    }
    const int v=rng()%in.n, old=s.power[v], oldend=s.end[v];
    if(!oldend)continue;
    int i;
    const unsigned u=rng()%10;
    if(u<2)i=0;
    else if(u<5)i=rng()%oldend;
    else i=max(0,oldend-1-int(rng()%min(12,oldend)));
    const int p=min(old-1,i>0?in.reaches[v][i-1].first:0);
    undo.clear();lost.clear();used.fill(false);
    auto edit=[&](int x,int r){
      if(!used[x])used[x]=true,undo.push_back({x,s.power[x]});
      s.set(x,r);
    };
    edit(v,p);
    for(int at=s.end[v];at<oldend;++at){
      int j=in.reaches[v][at].second;
      if(s.covered[j]==0)lost.push_back(j);
    }
    shuffle(lost.begin(),lost.end(),rng);
    bool legal=true;
    for(int j:lost)if(s.covered[j]==0){
      Int most=INF;int pick=-1;
      for(int partner:in.available[j])if(partner!=v){
        int cost=in.radius[partner][j];
        int was=s.power[partner];
        Int delta=Int(cost)*cost-Int(was)*was+(was==0 && partner!=0?in.activation[partner]/2:0);
        if(delta<most)most=delta,pick=partner;
      }
      if(pick<0){legal=false;break;}
      edit(pick,in.radius[pick][j]);
    }
    if(legal){
      const Int q=s.cost+in.mst(s.power,tree);
      if(q<=current || generate_canonical<double,32>(rng)<exp((current-q)/temp)){
        current=q;
        if(q<best)best=q,answer=s.power;
        continue;
      }
    }
    for(auto [x,r]:undo)s.set(x,r);
  }
  return answer;
}

vector<int> steiner(const Instance& in,const vector<int>& power,
                    const vector<int>& fallback,Int cost,Clock::time_point stop){
  vector<int> required{0};
  for(int i=1;i<in.n;++i)if(power[i]>0)required.push_back(i);
  vector<int> best=fallback;
  vector<int> candidate(in.m);
  array<int,MAXN> near{};
  array<Int,MAXN> distance{};
  array<bool,MAXN> on{};
  for(int seed:required){
    if(Clock::now()>=stop)break;
    fill(candidate.begin(),candidate.end(),0);
    near.fill(-1);distance.fill(INF);on.fill(false);
    auto attach=[&](int q){
      if(on[q])return;
      on[q]=true;
      for(int r:required)if(in.dist[q][r]<distance[r]){
        distance[r]=in.dist[q][r];near[r]=q;
      }
    };
    attach(seed);
    Int length=0;
    while(length<cost){
      int dest=-1;Int d=INF;
      for(int r:required)if(!on[r] && distance[r]<d)dest=r,d=distance[r];
      if(dest<0){cost=length;best=candidate;break;}
      for(int a=near[dest];a!=dest;){
        int e=in.next[a][dest];
        assert(e>=0);
        auto [x,y,w]=in.edges[e];
        if(!candidate[e])candidate[e]=1,length+=w;
        a=x+y-a;
        attach(a);
      }
    }
  }
  return best;
}
void print(ostream& out,const vector<int>& a){
  for(int i=0;i<int(a.size());++i)out<<(i?" ":"")<<a[i];
  out<<'\n';
}
} // namespace

#ifndef AHC020_NO_MAIN
int main(){
  const auto start=Clock::now();
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  Instance in{};
  in.initialize();
  auto power=find_power(in,start+chrono::milliseconds(1770));
  vector<int> baseline,ans;
  vector<unsigned char> scratch;
  Int c=in.mst(power,scratch,&baseline);
  ans=baseline;
#if AHC020_STEINER || defined(AHC020_DIAGNOSTIC)
  in.shortest_paths();
  ans=steiner(in,power,baseline,c,start+chrono::milliseconds(1880));
#endif
#ifdef AHC020_DIAGNOSTIC
  print(cerr,baseline);
#endif
  print(cout,power);
  print(cout,ans);
}
#endif
