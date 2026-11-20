#include <bits/stdc++.h>
#include "../../library/square-lattice-line-bits.hpp"
using namespace std;

#ifndef AHC014_REWIND
#define AHC014_REWIND 1
#endif
#ifndef AHC014_MS
#define AHC014_MS 4550
#endif

namespace {
using Clock=chrono::steady_clock;
constexpr auto DX=SquareLatticeLineBits::dx;
constexpr auto DY=SquareLatticeLineBits::dy;
struct Point {int x,y;};
Point shift(Point p,int d,int len){return {p.x+DX[d]*len,p.y+DY[d]*len};}

struct Move {
  Point base;
  int d,u,v;
  array<Point,4> corners() const {
    auto a=shift(base,d,u);
    const int e=(d+2)%8;
    auto b=shift(base,e,v);
    return {shift(a,e,v),a,base,b};
  }
};

struct State {
  int n;
  SquareLatticeLineBits marks,edges;
  vector<Point> marked;
  vector<Move> answer;
  int value=0;
  explicit State(int n):n(n),marks(n),edges(n) {}
  int weight(Point p) const {
    int c=(n-1)/2,x=p.x-c,y=p.y-c;
    return x*x+y*y+1;
  }
  void insert(Point p){marked.push_back(p);marks.mark(p.x,p.y);value+=weight(p);}
  bool legal(const Move& m) const {
    auto [add,a,b,c]=m.corners();
    if(!marks.inside(add.x,add.y) || marks.marked(add.x,add.y))return false;
    const int d=m.d,e=(d+2)%8;
    return !marks.inner_mark(a.x,a.y,e,m.v) && !marks.inner_mark(c.x,c.y,d,m.u)
       && !edges.overlaps_segment(b.x,b.y,d,m.u)
       && !edges.overlaps_segment(b.x,b.y,e,m.v)
       && !edges.overlaps_segment(a.x,a.y,e,m.v)
       && !edges.overlaps_segment(c.x,c.y,d,m.u);
  }
  void apply(const Move&m) {
    auto [add,a,b,c]=m.corners();
    const int d=m.d,e=(d+2)%8;
    edges.add_segment(b.x,b.y,d,m.u);
    edges.add_segment(b.x,b.y,e,m.v);
    edges.add_segment(a.x,a.y,e,m.v);
    edges.add_segment(c.x,c.y,d,m.u);
    insert(add);
    answer.push_back(m);
  }
};

struct Rng {
  uint64_t x=1445681429;
  uint32_t next(){x^=x<<13;x^=x>>7;x^=x<<17;return x>>24;}
  double unit(){return next()*(1.0/4294967296.0);}
};

optional<Move> choose(const State& s,const vector<double>& length_value,
                      const vector<double>& weights,double noise,Rng& rng) {
  double best=-1;
  optional<Move> ans;
  for(Point p:s.marked){
    array<int,8> reach;
    for(int d=0;d<8;++d)reach[d]=s.marks.nearest_steps(p.x,p.y,d);
    for(int d=0;d<8;++d){
      int u=reach[d],v=reach[(d+2)%8];
      if(!u||!v)continue;
      Move m{p,d,u,v};
      if(!s.legal(m))continue;
      const auto q=m.corners()[0];
      const double score=weights[q.x*s.n+q.y]*length_value[u+v]
                        *(1.0-noise+2*noise*rng.unit());
      if(score>best)best=score,ans=m;
    }
  }
  return ans;
}

struct Result {
  vector<Move> moves;
  int value,rest=0,rewind=0;
};
Result solve(const State& initial,Clock::time_point stop){
  Result best{{},initial.value};
  vector<double> weights(initial.n*initial.n);
  for(int x=0;x<initial.n;++x)for(int y=0;y<initial.n;++y)
    weights[x*initial.n+y]=sqrt(initial.weight({x,y})+0.035*initial.n*initial.n);
  constexpr double exps[]={1.30,1.65,1.00,0.75,1.95,2.25};
  array<vector<double>,size(exps)> length_values;
  for(int i=0;i<int(size(exps));++i){
    auto& v=length_values[i];
    v.resize(2*initial.n+1);
    for(int n=1;n<int(v.size());++n)v[n]=pow(n,-exps[i]);
  }
  Rng rng;
  for(int trial=0; Clock::now()<stop; ++trial){
    State s=initial;
    const bool rewind=AHC014_REWIND && best.moves.size()>=4 && trial%4!=0;
    if(rewind){
      const int k=best.moves.size();
      int keep=k*(trial%4==1?50+(rng.next()%40):trial%4==2?76+(rng.next()%20):15+(rng.next()%62))/100;
      for(int i=0;i<keep;++i)s.apply(best.moves[i]);
      ++best.rewind;
    }else{
      ++best.rest;
    }
    const double noise=trial==0?0.0:0.19+0.44*rng.unit();
    auto& length=length_values[trial%length_values.size()];
    while(Clock::now()<stop){
      auto m=choose(s,length,weights,noise,rng);
      if(!m)break;
      s.apply(*m);
    }
    if(s.value>best.value){
      best.value=s.value;
      best.moves=std::move(s.answer);
    }
  }
  return best;
}
#if defined(AHC014_CASCADE_DIAGNOSTIC) || defined(AHC014_CASCADE_TEST)
#include "cascade-regrowth.hpp"
#endif

void print_moves(std::ostream& out,const std::vector<Move>& moves) {
  out<<moves.size()<<'\n';
  for(auto step:moves){
    bool first=true;
    for(auto p:step.corners()){
      if(!first)out<<' ';
      first=false;
      out<<p.x<<' '<<p.y;
    }
    out<<'\n';
  }
}
} // namespace

#ifndef AHC014_NO_MAIN
int main(){
  const auto start=Clock::now();
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n,m;
  if(!(cin>>n>>m))return 0;
  State s(n);
  for(int i=0;i<m;++i){int x,y;cin>>x>>y;s.insert({x,y});}
#ifdef AHC014_CASCADE_DIAGNOSTIC
  auto frozen=solve(s,start+chrono::milliseconds(4270));
  auto cmp=compare_cascade(s,frozen,start+chrono::milliseconds(4810));
  cerr<<"frozen="<<frozen.value<<" suffix="<<cmp.suffix.value
      <<" cascade="<<cmp.cascade.value<<" completed="<<cmp.completed
      <<" deleted="<<cmp.deleted<<" preserved_after="<<cmp.preserved_after<<'\n';
  print_moves(cerr,frozen.moves);
  print_moves(cerr,cmp.suffix.moves);
  auto out=std::move(cmp.cascade);
#else
  auto out=solve(s,start+chrono::milliseconds(AHC014_MS));
#endif
  print_moves(cout,out.moves);
#ifdef AHC014_STATS
  cerr<<"weight="<<out.value<<" full="<<out.rest<<" rewind="<<out.rewind<<'\n';
#endif
}
#endif
