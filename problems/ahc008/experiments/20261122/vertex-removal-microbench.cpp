#pragma GCC optimize("O3")
#include <bits/stdc++.h>
#include "../../../../library/vertex-removal-components.hpp"
using namespace std;
constexpr int S=30, N=S*S;
using Graph = array<vector<int>,N>;
const Graph adj = [] {
  Graph g;
  for (int i=0;i<N;++i) {
    if(i>=S) g[i].push_back(i-S);
    if(i<N-S) g[i].push_back(i+S);
    if(i%S) g[i].push_back(i-1);
    if(i%S<S-1) g[i].push_back(i+1);
  }
  return g;
}();
struct Test { array<bool,N> active; array<int,N> weight; vector<int> candidates; };
uint64_t term(int size,int w) { return uint64_t(size)*size*1000003+uint64_t(w)*w*1000033; }
int main(){
  mt19937 rng(178965);
  vector<Test> tests(250);
  for (auto& t:tests) for(int i=0;i<N;++i){
    t.active[i] = rng()%8 > 0;
    t.weight[i] = rng()%31 == 0;
    if(t.active[i] && rng()%15==0)t.candidates.push_back(i);
  }
  for(int repeat=0;repeat<3;++repeat){
    uint64_t fast=0, slow=0;
    auto begin=chrono::steady_clock::now();
    VertexRemovalComponents<int> cut(N);
    for(const auto& t:tests){
      cut.build([](int v)->const auto& {return adj[v];},[&](int v){return t.active[v];},[&](int v){return t.weight[v];});
      for(int v:t.candidates)cut.after_removing(v,[&](int,int s,int w){fast+=term(s,w);});
    }
    auto mid=chrono::steady_clock::now();
    array<int,N> seen{},q{};
    int label=0;
    for(const auto& t:tests)for(int v:t.candidates){
      ++label; seen[v]=label;
      for(int a:adj[v]) if(t.active[a] && seen[a]!=label){
        int len=1, w=0; q[0]=a;seen[a]=label;
        for(int i=0;i<len;++i){
          int x=q[i];w+=t.weight[x];
          for(int y:adj[x])if(t.active[y]&&seen[y]!=label){seen[y]=label;q[len++]=y;}
        }
        slow+=term(len,w);
      }
    }
    auto end=chrono::steady_clock::now();
    assert(fast==slow);
    cout<<"fp "<<fast<<" fast "<<chrono::duration<double>(mid-begin).count()
        <<" slow "<<chrono::duration<double>(end-mid).count()<<'\n';
  }
}
