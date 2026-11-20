#include <bits/stdc++.h>
#include "../../library/stack-durability-reserve.hpp"
using namespace std;
#ifndef AHC049_BEAM
#define AHC049_BEAM 1
#endif
#ifndef AHC049_ANCHORS
#define AHC049_ANCHORS 1
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int N=20,V=N*N,MAX_BOX=14;
using Reserve=StackDurabilityReserve<MAX_BOX>;
struct Tour{
  Reserve reserve;
  array<int,MAX_BOX> box{};
  int size=0,detour=0,saving=0;
  int at()const{return box[size-1];}
};
struct Problem{
  mutable long long probes=0;
  array<int,V> weight,dur;
  array<array<uint8_t,V>,V> distance;
  array<vector<int>,V> near;
  static int home(int p){return p/N+p%N;}
  void read(){
    int n;cin>>n;assert(n==N);
    for(int& x:weight)cin>>x;
    for(int& x:dur)cin>>x;
    for(int p=0;p<V;++p)for(int q=1;q<V;++q){
      int d=abs(p/N-q/N)+abs(p%N-q%N);
      distance[p][q]=d;
      if(d>0&&d<=8)near[p].push_back(q);
    }
  }
  Tour first(int p)const{
    Tour t;
    bool ok=t.reserve.append(weight[p],dur[p],home(p),home(p));
    assert(ok);t.box[0]=p;t.size=1;
    return t;
  }
  bool append(Tour& t,int p,int detour_cap)const{
    if(t.size>=MAX_BOX||find(t.box.begin(),t.box.begin()+t.size,p)!=t.box.begin()+t.size)return false;
    int go=distance[t.at()][p],e=go+home(p)-home(t.at());
    if(t.detour+e>detour_cap||2*home(p)<=e)return false;
    if(!t.reserve.append(weight[p],dur[p],go,home(p)))return false;
    t.detour+=e;t.saving+=2*home(p)-e;
    t.box[t.size++]=p;
    return true;
  }
  Tour greed(int start,const array<char,V>& available)const{
    Tour current=first(start);
    for(;;){
      Tour pick;double gain=-1.;
      for(int p:near[current.at()])if(available[p]){
        auto can=current;
        if(!append(can,p,0))continue;
        double g=2*home(p)/(1.+sqrt(double(weight[p])))+(dur[p]>10000?.14:0.);
        if(g>gain){gain=g;pick=can;}
      }
      if(gain<0.)return current;
      current=pick;
    }
  }
  Tour search(int start,const array<char,V>& available,Clock::time_point deadline,int width=14)const{
    auto best=greed(start,available);
    vector<Tour> layer{first(start)};
    auto rank=[](const Tour& t){
      long long left=0;
      for(int i=0;i<t.size;++i)left+=min(15000LL,t.reserve.layer[i].margin);
      return 1.*t.saving+.03*home(t.at())+.00007*left;
    };
    for(int depth=1;depth<MAX_BOX&&Clock::now()<deadline;++depth){
      vector<Tour> next;
      for(const auto& t:layer)for(int p:near[t.at()])if(available[p]){
        ++probes;
        Tour q=t;
        if(!append(q,p,4))continue;
        if(q.saving>best.saving)best=q;
        next.push_back(q);
      }
      if(next.empty())break;
      auto cmp=[&](const auto& a,const auto& b){return rank(a)>rank(b);};
      if(int(next.size())>width){
        nth_element(next.begin(),next.begin()+width,next.end(),cmp);next.resize(width);
      }
      layer=std::move(next);
    }
    return best;
  }
  int alternate(int start,const array<char,V>& available)const{
    int best=-1;
    auto support=[&](int p){
      // Favor a bottom box that can carry more future cargo all the way home.
      return pair{dur[p]/home(p),home(p)};
    };
    for(int p:near[start])if(available[p]&&home(p)>=home(start)-2&&
        (best<0||support(p)>support(best)))best=p;
    return best;
  }
  pair<int,string> solve(bool beam,Clock::time_point deadline)const{
    array<char,V> remaining;
    remaining.fill(true);remaining[0]=false;
    string plan;
    int move=0,pos=0;
    auto route=[&](int target){
      // If both nonzero endpoints would cross the exit under vertical-first
      // traversal, take the same-length horizontal-first path instead.
      if(pos>0&&target>0&&pos%N==0&&target/N==0)
        for(;pos%N<target%N;++pos){plan+='R';++move;}
      for(;pos/N<target/N;pos+=N){plan+='D';++move;}
      for(;pos/N>target/N;pos-=N){plan+='U';++move;}
      for(;pos%N<target%N;++pos){plan+='R';++move;}
      for(;pos%N>target%N;--pos){plan+='L';++move;}
    };
    while(true){
      int start=-1;
      for(int p=1;p<V;++p)if(remaining[p]&&
          (start<0||pair{home(p),dur[p]}>pair{home(start),dur[start]}))start=p;
      if(start<0)break;
      auto trip=greed(start,remaining);
      if(beam&&Clock::now()<deadline){
        const int other=AHC049_ANCHORS?alternate(start,remaining):-1;
        trip=search(start,remaining,deadline,other<0?14:7);
        if(other>=0&&Clock::now()<deadline){
          auto q=search(other,remaining,deadline,7);
          if(q.saving>trip.saving)trip=q;
        }
      }
      for(int i=0;i<trip.size;++i){
        int p=trip.box[i];
        route(p);plan+='1';remaining[p]=false;
      }
      route(0);
    }
    assert(plan.size()<=2*N*N*N);
    return {move,std::move(plan)};
  }
};
}
int main(){
  const auto deadline=Clock::now()+chrono::milliseconds(1450);
  ios::sync_with_stdio(false);cin.tie(nullptr);
  Problem p;p.read();
  auto answer=p.solve(false,deadline);
  const int base=answer.first;
  if(AHC049_BEAM){
    auto q=p.solve(true,deadline);
    if(q.first<answer.first)answer=std::move(q);
  }
  cerr<<"base="<<base<<" moves="<<answer.first<<" probes="<<p.probes<<'\n';
  for(char c:answer.second)cout<<c<<'\n';
}
