#include <bits/stdc++.h>
#include "../../library/small-grid-bit-flood.hpp"
using namespace std;

#ifndef AHC015_ROLLOUT
#define AHC015_ROLLOUT 1
#endif

namespace {
using Clock=chrono::steady_clock;
constexpr int N=10,T=N*N;
using Flood=SmallGridBitFlood<N,N>;
using Board=array<unsigned char,T>;
using Flavors=array<int,T>;
constexpr string_view D="FBLR";
constexpr auto lanes=[] {
  array<array<array<int,N>,N>,4> q{};
  for(int a=0;a<N;++a)for(int b=0;b<N;++b){
    q[0][a][b]=b*N+a;
    q[1][a][b]=(N-1-b)*N+a;
    q[2][a][b]=a*N+b;
    q[3][a][b]=a*N+N-1-b;
  }
  return q;
}();

Board tilt(const Board& b,int dir){
  Board a{};
  for(const auto& lane:lanes[dir]){
    int j=0;
    for(int i:lane)if(b[i])a[lane[j++]]=b[i];
  }
  return a;
}

void place(Board& b,int rank,int color){
  for(auto& p:b)if(!p && --rank==0){p=color;return;}
  assert(false);
}

int value(const Board& b,int time){
  array<Flood::Mask,4> mask{};
  for(int i=0;i<T;++i) mask[b[i]] |= Flood::cell(i/N,i%N);
  int square=0, pairs=0;
  for(int c=1;c<=3;++c){
    auto m=mask[c];
    Flood::components(m,[&](auto region){
      int size=Flood::count(region);
      square+=size*size;
    });
    if(time<T-1){
      pairs+=Flood::count(m & (m<<1));
      pairs+=Flood::count(m & (m<<Flood::stride));
    }
  }
  const int multiplier=time<55?5:time<88?3:time<99?1:0;
  return square+multiplier*pairs;
}

pair<Board,int> greedy(const Board& b,int t){
  Board best{};
  int q=-1;
  for(int d=0;d<4;++d) {
    auto child=tilt(b,d);
    int now=value(child,t);
    if(now>q)q=now,best=child;
  }
  return {best,q};
}

// All future empty ranks are equally likely, and their count depends only on t.
// Every subtree has (remaining!) leaves, so compare expected values by numerators.
int exact_after(const Board& placed,int t,const Flavors& color){
  if(t==T-1)return value(placed,t);
  int best=0;
  for(int d=0;d<4;++d){
    auto child=tilt(placed,d);
    int score=0;
    for(int rank=1;rank<T-t;++rank){
      auto copy=child;
      place(copy,rank,color[t+1]);
      score+=exact_after(copy,t+1,color);
    }
    best=max(best,score);
  }
  return best;
}

struct Rng {
  uint64_t x=150091;
  unsigned next(){x^=x<<13;x^=x>>7;x^=x<<17;return x>>19;}
  int rank(int size) {
    return 1 + ((uint64_t(next())*size)>>32);
  }
};

int decision(const Board& b,int t,const Flavors& color,Rng& rng,Clock::time_point hard){
  array<Board,4> a;
  array<int,4> q{};
  for(int d=0;d<4;++d)a[d]=tilt(b,d),q[d]=value(a[d],t);
  if(t==T-1)return 0;
  if(Clock::now()>=hard)return int(max_element(q.begin(),q.end())-q.begin());
  array<int64_t,4> score{};
  if(t>=T-4){
    for(int d=0;d<4;++d)for(int rank=1;rank<T-t;++rank){
      auto child=a[d];
      place(child,rank,color[t+1]);
      score[d]+=exact_after(child,t+1,color);
    }
  }else{
    const auto stop=min(hard,Clock::now()+chrono::milliseconds(15));
    if(AHC015_ROLLOUT){
      constexpr int samples=26, horizon=6;
      const int h=min(horizon,T-1-t);
      for(int sample=0;sample<samples && Clock::now()<stop;++sample){
        array<int,horizon> ranks{};
        for(int i=0;i<h;++i)ranks[i]=rng.rank(T-1-t-i);
        for(int d=0;d<4;++d){
          auto child=a[d];
          int leaf=q[d],sum=0;
          for(int i=0;i<h;++i){
            place(child,ranks[i],color[t+i+1]);
            auto [next,result]=greedy(child,t+i+1);
            child=next;leaf=result;sum+=result;
          }
          score[d]+=int64_t(leaf)*h*5+sum;
        }
      }
    }else{
      const int empty=T-1-t;
      int step=37;
      while(gcd(step,empty)>1)++step;
      for(int i=0;i<empty && Clock::now()<stop;++i){
        int rank=1+(i*step+17*t)%empty;
        for(int d=0;d<4;++d){
          auto child=a[d];
          place(child,rank,color[t+1]);
          score[d]+=greedy(child,t+1).second;
        }
      }
    }
  }
  int index=0;
  for(int d=1;d<4;++d)if(pair{score[d],q[d]}>pair{score[index],q[index]})index=d;
  return index;
}
} // namespace

#ifndef AHC015_NO_MAIN
int main(){
  auto start=Clock::now();
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  Flavors f;
  for(int& c:f)if(!(cin>>c))return 0;
  Board b{};
  Rng rng;
  const auto hard=start+chrono::milliseconds(1790);
  for(int t=0;t<T;++t){
    int rank;
    if(!(cin>>rank))return 0;
    place(b,rank,f[t]);
    const int d=decision(b,t,f,rng,hard);
    b=tilt(b,d);
    cout<<D[d]<<endl;
  }
}
#endif
