#include <bits/stdc++.h>
#include "../../library/bounded-layer-shortest-path.hpp"
using namespace std;
#ifndef AHC024_WAVY
#define AHC024_WAVY 1
#endif
#ifndef AHC024_RESTARTS
#define AHC024_RESTARTS 2
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int C=101;
struct Board {
  int h,w;
  vector<int> v;
  int get(int y,int x) const {
    if(y<0||y>=h||x<0||x>=w)return 0;
    return v[y*w+x];
  }
  Board transpose()const{
    Board b{w,h,vector<int>(v.size())};
    for(int y=0;y<h;++y)for(int x=0;x<w;++x)b.v[x*h+y]=get(y,x);
    return b;
  }
  Board cut(const vector<int>& seam)const{
    Board b{h,w-1,{}};
    b.v.reserve(h*(w-1));
    for(int y=0;y<h;++y)for(int x=0;x<w;++x)if(x!=seam[y])b.v.push_back(get(y,x));
    return b;
  }
  int colored()const{return ranges::count_if(v,[](int i){return i!=0;});}
};

template<class F>
void adjacencies(const Board& b,F f){
  for(int y=0;y<b.h;++y)for(int x=0;x<b.w;++x){
    const int c=b.get(y,x);
    f(c,b.get(y+1,x));f(c,b.get(y,x+1));
    if(y==0)f(c,0);
    if(x==0)f(c,0);
  }
}
struct Topology {
  array<bitset<C>,C> original{};
  int m;
  explicit Topology(const Board& b,int colors):m(colors){
    adjacencies(b,[&](int a,int c){if(a!=c)original[a][c]=original[c][a]=true;});
  }
  bool permits(int a,int b)const{return a==b||original[a][b];}
  bool valid(const Board& b)const{
    array<bitset<C>,C> now{};
    adjacencies(b,[&](int a,int c){if(a!=c)now[a][c]=now[c][a]=true;});
    if(now!=original)return false;
    const int W=b.w+2,H=b.h+2;
    array<int,52*52> grid{};
    array<unsigned char,52*52> seen{};
    array<bool,C> connected{};
    array<int,52*52> queue;
    for(int y=0;y<b.h;++y)
      copy_n(b.v.begin()+y*b.w,b.w,grid.begin()+(y+1)*W+1);
    for(int s=0;s<W*H;++s)if(!seen[s]){
      int color=grid[s];
      if(connected[color])return false;
      connected[color]=true;
      int top=0,end=1;
      queue[0]=s;seen[s]=1;
      auto add=[&](int u){if(!seen[u]&&grid[u]==color)seen[u]=1,queue[end++]=u;};
      while(top<end){
        int v=queue[top++];
        if(v>=W)add(v-W);
        if(v+W<W*H)add(v+W);
        if(v%W)add(v-1);
        if(v%W+1<W)add(v+1);
      }
    }
    for(int i=1;i<=m;++i)if(!connected[i])return false;
    return true;
  }
};
void straight(Board& b,const Topology& topo,Clock::time_point stop,mt19937* permute=nullptr){
  for(bool changed=true;changed&&Clock::now()<stop;){
    changed=false;
    for(int axis=0;axis<2;++axis){
      if(b.w>1 && permute){
        vector<int> order(b.w);
        iota(order.begin(),order.end(),0);
        shuffle(order.begin(),order.end(),*permute);
        for(int x:order){
          if(Clock::now()>=stop)break;
          auto cut=b.cut(vector<int>(b.h,x));
          if(topo.valid(cut)){b=std::move(cut);changed=true;break;}
        }
      }else if(b.w>1)for(int x=0;x<b.w && Clock::now()<stop;++x){
        auto cut=b.cut(vector<int>(b.h,x));
        if(topo.valid(cut)){b=std::move(cut);changed=true;break;}
      }
      b=b.transpose();
    }
  }
}
void erode(Board& b,const Topology& topo,Clock::time_point stop){
  for(bool changed=true;changed&&Clock::now()<stop;){
    changed=false;
    for(int y=0;y<b.h && Clock::now()<stop;++y)for(int x=0;x<b.w;++x){
      auto& v=b.v[y*b.w+x];
      if(!v || !topo.permits(0,v))continue;
      if(b.get(y-1,x)&&b.get(y+1,x)&&b.get(y,x-1)&&b.get(y,x+1))continue;
      int old=v;v=0;
      if(topo.valid(b))changed=true;
      else v=old;
    }
  }
}

optional<vector<int>> wavy(const Board& b,const Topology& topo,
                           mt19937& rng,int trial){
  array<array<int,C>,C> counts{};
  adjacencies(b,[&](int a,int c){++counts[a][c];++counts[c][a];});
  array<int,C> area{};
  for(int c:b.v)++area[c];
  auto risk=[&](int a,int c){
    if(a==c)return 0;
    int k=counts[a][c];
    return k==1?10000:k==2?120:k<=4?30:3;
  };
  constexpr int maxjump=1;
  int center=rng()%b.w;
  const int gravity=trial%6==0?20:trial%6==1?5:0;
  vector<optional<int>> node(b.h*b.w);
  for(int y=0;y<b.h;++y)for(int x=0;x<b.w;++x){
    const int a=b.get(y,x-1),v=b.get(y,x),c=b.get(y,x+1);
    if(area[v]<=1 || !topo.permits(a,c))continue;
    node[y*b.w+x]=(a==c&&a==v?0:a==v||c==v?8:22)
                 +risk(a,v)+risk(v,c)+int(rng()%160)+gravity*abs(x-center);
  }
  auto crossing=[&](int y,int a,int c)->optional<int>{
    // All columns outside this narrow interval reuse existing vertical edges.
    const int lo=min(a,c),hi=max(a,c);
    int cost=0;
    for(int x=lo;x<hi;++x){
      int u=b.get(y-1,x+(x>=a)),v=b.get(y,x+(x>=c));
      if(!topo.permits(u,v))return nullopt;
    }
    for(int x=lo;x<=hi;++x)cost+=risk(b.get(y-1,x),b.get(y,x));
    return cost;
  };
  auto path=bounded_layer_shortest_path<int>(b.h,b.w,maxjump,
      [&](int y,int x){return node[y*b.w+x];},crossing);
  if(!path)return nullopt;
  return std::move(path->states);
}

int carve(Board& b,const Topology& topo,Clock::time_point stop,uint32_t seed=240818){
  mt19937 rng(seed);
  int accepted=0;
  for(int misses=0;misses<160 && Clock::now()<stop;++misses){
    if(b.w>2)if(auto seam=wavy(b,topo,rng,misses)){
      auto changed=b.cut(*seam);
      if(topo.valid(changed)){
        b=std::move(changed);
        straight(b,topo,stop);
        ++accepted;
        misses=-1;
      }
    }
    b=b.transpose();
  }
  return accepted;
}
struct Restarts { int completed=0; int distinct=0; };

Restarts multistart(const Board& initial,const Board& frozen,const Topology& topo,
                   Board& chosen,Clock::time_point stop,bool diverse) {
  set<vector<int>> distinct;
  Restarts info{};
  mt19937 rest(245081);
  constexpr int count=10;
  for(int j=0;j<count && Clock::now()<stop;++j){
    auto b=frozen;
    uint32_t seam_seed=rest();
    if(diverse){
      b=initial;
      mt19937 order(seam_seed^0xa42cd31u);
      straight(b,topo,stop,&order);
    }
    distinct.insert(b.v);
    carve(b,topo,stop,seam_seed);
    erode(b,topo,stop);
    if(b.colored()<chosen.colored())chosen=std::move(b);
    if(Clock::now()<stop)++info.completed;
  }
  info.distinct=int(distinct.size());
  return info;
}

void print(ostream& os,const Board& b,int n){
  for(int y=0;y<n;++y){
    for(int x=0;x<n;++x)os<<(x?" ":"")<<b.get(y,x);
    os<<'\n';
  }
}
} // namespace

#ifndef AHC024_NO_MAIN
int main(){
  const auto start=Clock::now();
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n,m;
  if(!(cin>>n>>m))return 0;
  Board initial{n,n,vector<int>(n*n)};
  for(int& x:initial.v)cin>>x;
  const Topology topo(initial,m);
  auto frozen=initial;
  straight(frozen,topo,start+chrono::milliseconds(260));
  auto base=frozen;
  erode(base,topo,start+chrono::milliseconds(550));
  auto out=base;
  int carved=0;
#if AHC024_WAVY || defined(AHC024_DIAGNOSTIC)
  auto variant=frozen;
  carved=carve(variant,topo,start+chrono::milliseconds(1450));
  erode(variant,topo,start+chrono::milliseconds(1820));
  if(variant.colored()<out.colored())out=std::move(variant);
#endif
#if AHC024_RESTARTS || defined(AHC024_RESTART_STATS)
  int first_score=n*n-out.colored()+1;
  auto rest=multistart(initial,frozen,topo,out,start+chrono::milliseconds(1810),AHC024_RESTARTS==2);
#ifdef AHC024_RESTART_STATS
  cerr<<"first="<<first_score<<" restarts="<<rest.completed<<" roots="<<rest.distinct<<'\n';
#endif
#endif
  assert(topo.valid(out));
#ifdef AHC024_DIAGNOSTIC
  cerr<<"carved="<<carved<<" frozen_area="<<frozen.h*frozen.w<<'\n';
  print(cerr,base,n);
#endif
  print(cout,out,n);
}
#endif
