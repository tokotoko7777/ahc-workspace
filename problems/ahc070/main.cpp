#include <bits/stdc++.h>
#include "../../library/incremental-manhattan-distance.hpp"
using namespace std;
#ifndef AHC070_SPREAD
#define AHC070_SPREAD 1
#endif
#ifndef AHC070_CONDITIONAL
#define AHC070_CONDITIONAL 0
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int N=100,S=N*N,M=3;
using Shifts=array<pair<int,int>,M>;
struct Visit{array<int,S> direction;Shifts shifts;long long danger=LLONG_MAX;};
struct Solver{
  array<int,S> event;
  void read(){
    int n,m;cin>>n>>m;assert(n==N&&m==M);
    for(int& p:event){int y,x;cin>>y>>x;p=N*y+x;}
  }
  static int md(int p,int q){return abs(p/N-q/N)+abs(p%N-q%N);}
  static int move(int p,pair<int,int> d){
    return ((p/N+d.first)%N)*N+(p%N+d.second)%N;
  }
  Visit travel(Shifts shifts,int seed,bool conditional=false)const{
    Visit ans;
    ans.shifts=shifts;ans.danger=0;
    IncrementalManhattanDistance field(N,N);
    mt19937 rng(seed);
    array<array<int,M>,S> next;
    for(int p=0;p<S;++p)for(int k=0;k<M;++k)next[p][k]=move(p,shifts[k]);
    int pos=0;
    for(int t=0;t<S;++t){
      auto merit=[&](int v,int from,int horizon,int first=-1){
        const int d=first<0?field.distance(v):min(field.distance(v),md(v,first));
        if(d==0)return 0;
        int benefit=11*d+80;
        for(int i=from;i<min(S,from+horizon);++i){
          const int q=event[i],old=(first<0?field.distance(q):min(field.distance(q),md(q,first)));
          const int improvement=max(0,old-md(v,q));
          benefit+=(i==from?118:8)*improvement;
        }
        return benefit;
      };
      int best=INT_MIN,pick=0;
      for(int a=0;a<M;++a){
        const int q=next[pos][a];
        int two=0;
        for(int v:next[q])two=max(two,merit(v,t+1,7,conditional?q:-1));
        const int value=merit(q,t,15)+two/5+int(rng()%24);
        if(value>best){best=value;pick=a;}
      }
      pos=next[pos][pick];
      ans.direction[t]=pick;field.add(pos);
      const long long d=field.distance(event[t]);
      ans.danger+=int(sqrt(double(d*d*(t+1LL))));
    }
    return ans;
  }
  static Shifts triple(int x,int y){return {{{0,1},{1,0},{x,y}}};}
  void solve(Clock::time_point cutoff)const{
    auto best=travel(triple(1,1),701);
    const auto frozen=best.danger;
    const array<pair<int,int>,8> nearby{{
        {0,99},{99,0},{1,99},{99,1},{99,99},{1,2},{2,1},{2,3}}};
    const array<pair<int,int>,8> remote{{
        {17,39},{31,47},{43,67},{21,52},{27,37},{48,13},{63,83},{72,43}}};
    int trials=0;
    for(int i=0;i<8&&Clock::now()<cutoff;++i){
      auto d=(AHC070_SPREAD?remote:nearby)[i];
      auto ans=travel(triple(d.first,d.second),1707+i,AHC070_CONDITIONAL);
      if(ans.danger<best.danger)best=std::move(ans);
      ++trials;
    }
    cerr<<"frozen="<<frozen<<" danger="<<best.danger<<" trials="<<trials<<'\n';
    for(auto [y,x]:best.shifts)cout<<y<<' '<<x<<'\n';
    for(int a:best.direction)cout<<a<<'\n';
  }
};
}
int main(){
  const auto cutoff=Clock::now()+chrono::milliseconds(1450);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Solver s;s.read();s.solve(cutoff);
}
