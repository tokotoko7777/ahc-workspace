#include <algorithm>
#include <cassert>
#include <random>
#include "../library/dijkstra.hpp"
#include "../library/implicit-multi-source-dijkstra.hpp"

int main(){
  std::mt19937 rng(180422);
  for(int t=0;t<500;++t){
    const int n=1+rng()%36, inf=10000000;
    std::vector<std::vector<std::pair<int,int>>> graph(n+1);
    for(int a=0;a<n;++a)for(int b=0;b<n;++b)
      if(a!=b&&rng()%8==0)add_directed_edge(graph,a,b,int(rng()%15));
    std::vector<int> roots;
    for(int j=0;j<n;++j)if(rng()%7==0)roots.push_back(j),add_directed_edge(graph,n,j,0);
    if(!roots.empty())roots.push_back(roots[0]);
    std::vector<bool> goals(n);
    for(int j=0;j<n;++j)goals[j]=rng()%3==0;
    auto ref=dijkstra(graph,n,inf);
    int best=inf;
    for(int j=0;j<n;++j)if(goals[j])best=std::min(best,ref.distance[j]);
    auto actual=implicit_multi_source_dijkstra<int>(
        n,roots,inf,[&](int v,auto emit){for(auto [u,w]:graph[v])emit(u,w);},
        [&](int v){return goals[v];});
    assert(bool(actual)==(best<inf));
    if(actual){
      assert(actual->distance==best);
      const auto& p=actual->nodes;
      assert(std::find(roots.begin(),roots.end(),p.front())!=roots.end());
      assert(goals[p.back()]);
      int cost=0;
      for(int i=1;i<int(p.size());++i){
        int amount=inf;
        for(auto [v,w]:graph[p[i-1]])if(v==p[i])amount=std::min(amount,w);
        assert(amount<inf);
        cost+=amount;
      }
      assert(cost==best);
    }
  }
}
