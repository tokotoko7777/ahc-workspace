#pragma once
#include <algorithm>
#include <cassert>
#include <functional>
#include <optional>
#include <queue>
#include <utility>
#include <vector>

template<class Cost>
struct ImplicitShortestPath {
  Cost distance;
  std::vector<int> nodes; // root → goal; roots all have initial cost zero
};

// enumerate(v, emit) は全ての隣接先に対し emit(to, non_negative_cost) を呼ぶ。
// 目標に初めて確定した時点で停止し、グラフの隣接リストは構築しない。
template<class Cost,class Range,class Neighbors,class Goal>
std::optional<ImplicitShortestPath<Cost>> implicit_multi_source_dijkstra(
    int n,const Range& roots,Cost infinity,Neighbors enumerate,Goal goal) {
  std::vector<int> parent(n,-1);
  std::vector<Cost> dist(n,infinity);
  using Item=std::pair<Cost,int>;
  std::priority_queue<Item,std::vector<Item>,std::greater<Item>> pq;
  for(int v:roots){
    assert(v>=0&&v<n);
    if(parent[v]>=0)continue;
    parent[v]=v;
    dist[v]=Cost{};
    pq.emplace(Cost{},v);
  }
  while(!pq.empty()){
    auto [d,v]=pq.top();
    pq.pop();
    if(d!=dist[v])continue;
    if(goal(v)){
      std::vector<int> path{v};
      while(parent[path.back()]!=path.back()) path.push_back(parent[path.back()]);
      std::reverse(path.begin(),path.end());
      return ImplicitShortestPath<Cost>{d,std::move(path)};
    }
    enumerate(v,[&](int u,Cost cost){
      assert(u>=0&&u<n&&!(cost<Cost{}));
      if(d+cost<dist[u])dist[u]=d+cost,parent[u]=v,pq.emplace(d+cost,u);
    });
  }
  return std::nullopt;
}
