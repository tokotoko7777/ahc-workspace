#ifndef AHC026_BEAM
#define AHC026_BEAM 0
#endif
#if !AHC026_BEAM && !defined(AHC026_NO_MAIN)
// Selected default: the repository owner's unmodified original rollout example.
#include "../../examples/search/ahc026_deterministic_rollout.cpp"
#else
#include <bits/stdc++.h>
#include "../../library/simple-beam-search.hpp"
using namespace std;
#ifndef AHC026_SPLIT
#define AHC026_SPLIT 0
#endif
namespace {
using Clock=chrono::steady_clock;
constexpr int N=200,M=10;
using Op=pair<int,int>;
struct Profile {int begin,end,minimum;};
struct StackMap {
  array<uint8_t,N> data{};
  array<uint8_t,M> size{};
  int left=N;
  array<Profile,M> profile()const{
    array<Profile,M> out{};
    for(int i=0,p=0;i<M;++i){
      int end=p+size[i],m=N+1;
      for(int j=p;j<end;++j)m=min(m,int(data[j]));
      out[i]={p,end,m};p=end;
    }
    return out;
  }
  pair<int,int> locate(int x,const array<Profile,M>& bounds)const{
    for(int s=0;s<M;++s)for(int i=bounds[s].begin;i<bounds[s].end;++i)
      if(data[i]==x)return {s,i};
    assert(false);
    return {-1,-1};
  }
  int move(int from,int start,int to,const array<Profile,M>& p){
    assert(from!=to && p[from].begin<=start && start<p[from].end);
    const int end=p[from].end,len=end-start,insert=p[to].end;
    if(from<to)rotate(data.begin()+start,data.begin()+end,data.begin()+insert);
    else rotate(data.begin()+insert,data.begin()+start,data.begin()+end);
    size[from]-=len;
    size[to]+=len;
    return len+1;
  }
  void remove_top(int from,int start){
    int end=0;
    for(int i=0;i<=from;++i)end+=size[i];
    assert(end-1==start);
    memmove(data.data()+start,data.data()+start+1,left-start-1);
    --size[from];--left;
  }
};
uint64_t mix(uint64_t x){
  x^=x>>30;x*=0xbf58476d1ce4e5b9ULL;
  x^=x>>27;x*=0x94d049bb133111ebULL;
  return x^ (x>>31);
}
struct State {
  StackMap board;
  int energy=0,trace=0,estimate=0;
  uint64_t key=0;
  void assess(){
    int burden=0;
    uint64_t sum=0;
    for(int s=0,at=0;s<M;++s){
      int min_below=N+1, end=at+board.size[s];
      bool gap=true;
      uint64_t hash=137;
      for(;at<end;++at){
        int v=board.data[at];
        hash=hash*0x9e3779b97f4a7c15ULL+v;
        if(v>min_below){
          burden+=64+min(25,(v-min_below)/4)+int(gap)*31;
          gap=false;
        }else{
          gap=true;min_below=v;
        }
      }
      sum+=mix(hash);
    }
    estimate=energy*64+burden;
    key=sum;
  }
};
struct Trace {int parent;array<Op,3> op;int len;};
struct Plan {vector<Op> sequence;int energy,depth;};

pair<int,int> stow_score(const StackMap& b,int begin,int end,int below){
  int predicted=0;
  bool group=true;
  for(int at=begin;at<end;++at){
    int v=b.data[at];
    if(v>below){
      predicted+=64+min(25,(v-below)/4)+int(group)*31;
      group=false;
    }else below=v,group=true;
  }
  return {predicted,below};
}
template<class Emit>
void expand(const State& s,int target,bool split,vector<Trace>& history,Emit emit){
  const auto p=s.board.profile();
  const auto [from,at]=s.board.locate(target,p);
  const int end=p[from].end;
  auto child=[&](int upper,int top_dest,int low_dest){
    State z=s;
    auto& b=z.board;
    Trace trace{s.trace,{},0};
    if(top_dest>=0){
      trace.op[trace.len++]={b.data[upper],top_dest+1};
      z.energy+=b.move(from,upper,top_dest,p);
    }
    auto q=top_dest<0?p:b.profile();
    auto t=b.locate(target,q);
    assert(t.first==from);
    if(low_dest>=0){
      trace.op[trace.len++]={b.data[t.second+1],low_dest+1};
      z.energy+=b.move(from,t.second+1,low_dest,q);
      q=b.profile();t=b.locate(target,q);
    }
    b.remove_top(from,t.second);
    trace.op[trace.len++]={target,0};
    z.trace=int(history.size());
    history.push_back(trace);
    z.assess();
    emit(std::move(z));
  };
  if(at+1==end){
    child(-1,-1,-1);
    return;
  }
  for(int dst=0;dst<M;++dst)if(dst!=from)child(-1,-1,dst);
  if(!split||at+2>=end)return;
  using Split=tuple<int,int,int,int>;
  vector<Split> list;
  for(int cut=at+2;cut<end;++cut){
    array<pair<int,int>,M> stow{};
    for(int d=0;d<M;++d)if(d!=from)
      stow[d]=stow_score(s.board,cut,end,p[d].minimum);
    array<pair<int,int>,M> lower{};
    for(int d=0;d<M;++d)if(d!=from)
      lower[d]=stow_score(s.board,at+1,cut,p[d].minimum);
    for(int a=0;a<M;++a)if(a!=from)for(int b=0;b<M;++b)if(b!=from){
      int bound=a==b?stow_score(s.board,at+1,cut,stow[a].second).first:lower[b].first;
      list.emplace_back(stow[a].first+bound,cut,a,b);
    }
  }
  const int remain=min(12,int(list.size()));
  partial_sort(list.begin(),list.begin()+remain,list.end());
  for(int i=0;i<remain;++i){
    auto [q,cut,a,b]=list[i];
    child(cut,a,b);
  }
}

Plan solve(State initial,Clock::time_point stop,bool split){
  SimpleBeamSearch<State,int> search(initial,50,false);
  vector<Trace> trace(1);
  trace.reserve(160000);
  int target=1;
  for(;target<=N && Clock::now()<stop;++target){
    bool ok=search.step_each_with_key([&](const State& z,auto emit){
      expand(z,target,split,trace,emit);
    },[](const State& z){return z.estimate;},
      [](const State& z){return z.key;});
    assert(ok);
  }
  State finish=search.best();
  vector<Op> sequence;
  for(int node=finish.trace;node;node=trace[node].parent){
    auto& v=trace[node];
    for(int j=v.len-1;j>=0;--j)sequence.push_back(v.op[j]);
  }
  reverse(sequence.begin(),sequence.end());
  // Hard deadline fallback: move the blockers as one chunk and remove each box.
  int depth=target-1;
  for(;target<=N;++target){
    auto p=finish.board.profile();
    auto [from,at]=finish.board.locate(target,p);
    if(at+1<p[from].end){
      int dest=-1,val=-1;
      for(int j=0;j<M;++j)if(j!=from&&p[j].minimum>val)dest=j,val=p[j].minimum;
      sequence.push_back({finish.board.data[at+1],dest+1});
      finish.energy+=finish.board.move(from,at+1,dest,p);
      p=finish.board.profile();
      tie(from,at)=finish.board.locate(target,p);
    }
    finish.board.remove_top(from,at);
    sequence.push_back({target,0});
  }
  return {std::move(sequence),finish.energy,depth};
}
} // namespace

#ifndef AHC026_NO_MAIN
int main(){
  const auto start=Clock::now();
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  int n,m;
  if(!(cin>>n>>m))return 0;
  assert(n==N&&m==M);
  State initial{};
  initial.board.size.fill(n/m);
  for(auto& b:initial.board.data){int x;cin>>x;b=x;}
  auto plan=solve(initial,start+chrono::milliseconds(1680),AHC026_SPLIT);
  for(auto [v,to]:plan.sequence)cout<<v<<' '<<to<<'\n';
#ifdef AHC026_STATS
  cerr<<"cost="<<plan.energy<<" depth="<<plan.depth<<'\n';
#endif
}
#endif
#endif
