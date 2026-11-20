#include <bits/stdc++.h>
#include "../../library/degree-two-components.hpp"
using namespace std;

#ifndef AHC010_COORDINATED
#define AHC010_COORDINATED 1
#endif
#ifndef AHC010_LIMIT_MS
#define AHC010_LIMIT_MS 1810
#endif

namespace {
constexpr int H=30, W=15, N=H*W;
using Clock=chrono::steady_clock;
constexpr int to[8][4]={{1,0,-1,-1},{3,-1,-1,0},{-1,-1,3,2},{-1,2,1,-1},
                        {1,0,3,2},{3,2,1,0},{2,-1,0,-1},{-1,3,-1,1}};
constexpr int rotate_tile[8]={1,2,3,0,5,4,7,6};
constexpr int dr[4]={0,-1,0,1}, dc[4]={-1,0,1,0};

struct Geometry {
  array<array<int,4>,8> segment{};
  array<array<array<int,2>,2>,8> endpoints{};
  array<int,8> size{};
  Geometry() {
    for(int t=0;t<8;++t){
      segment[t].fill(-1);
      for(int d=0;d<4;++d) if(to[t][d]>d){
        int i=size[t]++;
        segment[t][d]=segment[t][to[t][d]]=i;
        endpoints[t][i]={d,to[t][d]};
      }
    }
  }
};
const Geometry geo;

struct Board {
  array<int,N> input{}, rotation{}, orientation{}, period{};
  array<array<int,4>,N> adj{};
  array<array<int,2>,2*N> edges{};
  DegreeTwoComponents scanner{2*N};

  void initialize() {
    for(int i=0;i<N;++i){
      period[i]=input[i]<4?4:2;
      for(int d=0;d<4;++d){
        int r=i/W+dr[d], c=i%W+dc[d];
        adj[i][d]=(0<=r && r<H && 0<=c && c<W) ? r*W+c : -1;
      }
    }
  }
  void store(int i,int r){
    rotation[i]=r;
    int t=input[i];
    while(r--) t=rotate_tile[t];
    orientation[i]=t;
  }
  void build_links(int i){
    int t=orientation[i];
    for(int j=0;j<geo.size[t];++j) for(int side=0;side<2;++side){
      int d=geo.endpoints[t][j][side], u=adj[i][d], next=-1;
      if(u>=0){
        int q=geo.segment[orientation[u]][d^2];
        if(q>=0) next=2*u+q;
      }
      edges[2*i+j][side]=next;
    }
  }
  void refresh(int i) {
    build_links(i);
    for(int u:adj[i]) if(u>=0) build_links(u);
  }
  pair<double,int> quality(){
    int closed=0, sq=0, link=0;
    double dominant=0;
    scanner.scan([&](int v){return v%2<geo.size[orientation[v/2]];},
                 [&](int v){return edges[v];},
                 [&](int len,int a,int b){
       sq+=len*len;
       link+=len-(a>=0);
       if(a<0) {
         closed=max(closed,len);
         dominant=max(dominant,1.04*len);
       }else{
         a/=2;b/=2;
         int dist=abs(a/W-b/W)+abs(a%W-b%W);
         dominant=max(dominant,0.97*len-0.7*dist);
       }
    });
    return {dominant + 0.00065*sq + 0.035*link, closed};
  }
  template<class Rng>
  int supports(int cell,int d,Rng& rng) const {
    array<int,4> option{};
    int t=input[cell], count=0;
    for(int r=0;r<period[cell];++r,t=rotate_tile[t]){
      if(geo.segment[t][d]>=0) option[count++]=r;
    }
    assert(count);
    return option[rng()%count];
  }
};

pair<array<int,N>,int> solve(Board& b,Clock::time_point stop, uint32_t seed){
  mt19937 rng(seed);
  auto best_rot=b.rotation;
  int best_length=0;
  double best_quality=-1e100;
  auto begin=Clock::now();
  constexpr int restarts=3;
  for(int restart=0;restart<restarts;++restart){
    if(Clock::now()>=stop) break;
    auto stage_start=Clock::now();
    auto stage_stop=begin+(stop-begin)*(restart+1)/restarts;
    for(int i=0;i<N;++i) b.store(i,rng()%b.period[i]);
    for(int i=0;i<N;++i) b.build_links(i);
    auto [current,closed]=b.quality();
    auto update=[&](double val,int cyc){
      if(cyc>best_length || (cyc==best_length && val>best_quality))
        best_length=cyc,best_quality=val,best_rot=b.rotation;
    };
    update(current,closed);
    double temp=6.0;
    for(int it=0;;++it){
      if((it&31)==0){
        auto t=Clock::now();
        if(t>=stage_stop) break;
        double ratio=double((t-stage_start).count())/(stage_stop-stage_start).count();
        temp=6.0*pow(0.045,min(1.0,ratio));
      }
      int u=rng()%N, v=-1, new_u, new_v=-1;
      if(AHC010_COORDINATED && rng()%100<48){
        int d=rng()%4; v=b.adj[u][d];
        if(v<0) continue;
        new_u=b.supports(u,d,rng);
        new_v=b.supports(v,d^2,rng);
      }else{
        new_u=(b.rotation[u]+1+rng()%(b.period[u]-1))%b.period[u];
      }
      int old_u=b.rotation[u], old_v=v<0?-1:b.rotation[v];
      if(new_u==old_u && (v<0 || new_v==old_v)) continue;
      b.store(u,new_u);
      if(v>=0) b.store(v,new_v);
      b.refresh(u);
      if(v>=0) b.refresh(v);
      auto [q,loop]=b.quality();
      if(q>=current || generate_canonical<double,32>(rng)<exp((q-current)/temp)){
        current=q;
        update(q,loop);
      }else{
        b.store(u,old_u);
        if(v>=0) b.store(v,old_v);
        b.refresh(u);
        if(v>=0) b.refresh(v);
      }
    }
  }
  return {best_rot,best_length};
}
} // namespace

int main(){
  const auto start=Clock::now();
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  array<Board,2> b;
  for(int i=0;i<H;++i){
    string row;
    if(!(cin>>row)) return 0;
    for(int j=0;j<2*W;++j) b[j/W].input[i*W+j%W]=row[j]-'0';
  }
  for(auto& board:b) board.initialize();
  auto [left,l]=solve(b[0],start+chrono::milliseconds(AHC010_LIMIT_MS/2),1739);
  auto [right,r]=solve(b[1],start+chrono::milliseconds(AHC010_LIMIT_MS),7181);
  for(int i=0;i<H;++i){
    for(const auto& p:{cref(left),cref(right)})
      for(int j=0;j<W;++j) cout<<p.get()[i*W+j];
  }
  cout<<'\n';
#ifdef AHC010_STATS
  cerr<<"left="<<l<<" right="<<r<<'\n';
#endif
}
