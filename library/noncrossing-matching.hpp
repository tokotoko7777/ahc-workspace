#ifndef AHC_NONCROSSING_MATCHING_HPP
#define AHC_NONCROSSING_MATCHING_HPP
#include <algorithm>
#include <numeric>
#include <vector>

// Pick a max-weight subset of two-side edges whose endpoints are strictly
// increasing on both sides. Each input edge has fields left, right, weight.
// Output contains original indexes in order; empty is allowed. O(E^2) time,
// O(E) scratch. Useful when the ready-set is small, e.g. parallel jobs.
template<class Edges>
std::vector<int> maximum_weight_noncrossing_matching(const Edges& edge){
  using Weight=decltype(edge[0].weight);
  const int n=int(edge.size());
  std::vector<int> order(n),parent(n,-1);
  std::vector<Weight> value(n);
  std::iota(order.begin(),order.end(),0);
  std::stable_sort(order.begin(),order.end(),[&](int a,int b){return edge[a].left<edge[b].left;});
  int best=-1;Weight total{};
  for(int i=0;i<n;++i){
    const auto& x=edge[order[i]];
    value[i]=x.weight;
    for(int j=0;j<i;++j){
      const auto& y=edge[order[j]];
      if(y.left<x.left&&y.right<x.right&&value[j]+x.weight>value[i]){
        value[i]=value[j]+x.weight;parent[i]=j;
      }
    }
    if(value[i]>total){total=value[i];best=i;}
  }
  std::vector<int> selected;
  for(int j=best;j>=0;j=parent[j])selected.push_back(order[j]);
  std::reverse(selected.begin(),selected.end());
  return selected;
}
#endif
