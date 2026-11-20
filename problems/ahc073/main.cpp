#include <bits/stdc++.h>
#include "../../library/reflection-launch-range.hpp"
using namespace std;
#ifndef AHC073_BEAM
#define AHC073_BEAM 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int N=128,M=150,K=3;
using Rack=array<int,K>;
struct Move{int launch=-1,center=-1;};
struct Node{
  Rack circle{};
  int cost=0,rank=0,parent=-1;
  Move leg;
};
struct Plan{
  int cost=INT_MAX;
  Rack initial{};
  vector<Move> leg;
};
struct Problem{
  int t;
  vector<int> p;
  mutable long long work=0;
  void read(){
    int n,m,k;cin>>n>>m>>k>>t;assert(n==N&&m==M&&k==K);
    for(int i=0;i<=M;++i){int a,b;cin>>a>>b;p.push_back(a*N+b);}
  }
  static int md(int a,int b){return abs(a/N-b/N)+abs(a%N-b%N);}
  static int reflect(int x,int c){
    const int y=2*(c/N)-x/N,z=2*(c%N)-x%N;
    assert(0<=y&&y<N&&0<=z&&z<N);
    return y*N+z;
  }
  static uint64_t key(const Rack& rack){
    return (uint64_t(rack[0])<<28)|(uint64_t(rack[1])<<14)|rack[2];
  }
  Rack starts(int trial)const{
    Rack result{};
    mt19937 rng(730800+trial);
    for(int i=0;i<K;++i){
      const int j=trial?int(rng()%8):i;
      int y=(p[j]/N+p[j+1]/N)/2,x=(p[j]%N+p[j+1]%N)/2;
      if(trial){
        y=clamp(y+int(rng()%5)-2,0,N-1);
        x=clamp(x+int(rng()%5)-2,0,N-1);
      }
      result[i]=y*N+x;
    }
    ranges::sort(result);return result;
  }
  int future_saving(const Rack& circle,int step)const{
    if(step+1>=M)return 0;
    int a=p[step+1],b=p[step+2],base=md(a,b),saving=0;
    for(int c:circle){
      const auto [y,z]=reflection_launch_range(N,a/N,b/N,c/N);
      const auto [x,w]=reflection_launch_range(N,a%N,b%N,c%N);
      const int launch=N*y+x;
      saving=max(saving,base-1-md(a,launch)-md(reflect(launch,c),b));
    }
    return saving;
  }
  Plan search(Rack initial,int width,Clock::time_point deadline)const{
    vector<vector<Node>> layers(M+1);
    layers[0].push_back({initial,0,0,-1,{}});
    for(int step=0;step<M;++step){
      if(Clock::now()>=deadline)return {};
      unordered_map<uint64_t,int> visited;
      vector<Node> next;
      auto& cur=layers[step];
      const int a=p[step],b=p[step+1],walking=md(a,b);
      auto add=[&](Rack rack,int price,int prev,Move leg){
        ++work;
        ranges::sort(rack);
        const int cost=cur[prev].cost+price;
        const int rank=8*cost-3*future_saving(rack,step);
        const auto hash=key(rack);
        auto [it,created]=visited.try_emplace(hash,int(next.size()));
        if(created)next.push_back({rack,cost,rank,prev,leg});
        else if(cost<next[it->second].cost)next[it->second]={rack,cost,rank,prev,leg};
      };
      for(int i=0;i<int(cur.size());++i){
        add(cur[i].circle,walking,i,{});
        for(int k=0;k<K;++k){
          const int c=cur[i].circle[k];
          const auto [y,z]=reflection_launch_range(N,a/N,b/N,c/N);
          const auto [x,w]=reflection_launch_range(N,a%N,b%N,c%N);
          const int sample=N*y+x;
          const int price=md(a,sample)+1+md(reflect(sample,c),b);
          if(price>walking+3)continue;
          const int fy=step+1<M?(b/N+p[step+2]/N)/2:(y+z)/2;
          const int fx=step+1<M?(b%N+p[step+2]%N)/2:(x+w)/2;
          const int trials[]={N*y+x,N*y+w,N*z+x,N*z+w,N*((y+z)/2)+(x+w)/2,
                              N*clamp(fy,y,z)+clamp(fx,x,w)};
          array<int,6> used{};
          int count=0;
          for(int launch:trials){
            if(find(used.begin(),used.begin()+count,launch)!=used.begin()+count)continue;
            used[count++]=launch;
            auto circles=cur[i].circle;circles[k]=launch;
            add(circles,price,i,{launch,c});
          }
        }
      }
      auto cmp=[](const Node& a,const Node& b){
        return tuple{a.rank,a.cost,a.circle}<tuple{b.rank,b.cost,b.circle};
      };
      if(int(next.size())>width){
        nth_element(next.begin(),next.begin()+width,next.end(),cmp);
        next.resize(width);
      }
      ranges::sort(next,cmp);layers[step+1]=move(next);
    }
    int id=0;
    Plan result{layers[M][id].cost,initial,vector<Move>(M)};
    for(int i=M;i;--i){auto& a=layers[i][id];result.leg[i-1]=a.leg;id=a.parent;}
    return result;
  }
  void print(const Plan& plan)const{
    for(int c:plan.initial)cout<<c/N<<' '<<c%N<<'\n';
    int pos=p[0],steps=0;
    auto walk=[&](int target){
      while(pos/N<target/N){cout<<"D\n";pos+=N;++steps;}
      while(pos/N>target/N){cout<<"U\n";pos-=N;++steps;}
      while(pos%N<target%N){cout<<"R\n";++pos;++steps;}
      while(pos%N>target%N){cout<<"L\n";--pos;++steps;}
    };
    auto circle=plan.initial;
    for(int i=0;i<M;++i){
      auto [launch,center]=plan.leg[i];
      if(launch>=0){
        walk(launch);
        auto found=ranges::find(circle,center);assert(found!=circle.end());
        *found=pos;
        cout<<"J "<<center/N<<' '<<center%N<<'\n';
        pos=reflect(pos,center);++steps;
      }
      walk(p[i+1]);
    }
    assert(steps==plan.cost&&steps<=t);
  }
  void run(Clock::time_point deadline)const{
    auto initial=starts(0);
    Plan best{t,initial,vector<Move>(M)};
    auto accept=[&](Plan next){if(next.cost<best.cost)best=move(next);};
    accept(search(initial,1,deadline));
    const int base=best.cost;
    if(AHC073_BEAM)accept(search(initial,24,deadline));
    else for(int i=1;i<=24&&Clock::now()<deadline;++i)accept(search(starts(i),1,deadline));
    cerr<<"base="<<base<<" moves="<<best.cost<<" candidates="<<work<<'\n';
    print(best);
  }
};
}
int main(){
  const auto deadline=Clock::now()+chrono::milliseconds(1400);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();p.run(deadline);
}
