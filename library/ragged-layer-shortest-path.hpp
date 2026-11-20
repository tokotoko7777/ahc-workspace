#pragma once
#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

// Shortest path through nonempty consecutive layers of different widths.
// initial(node) is the first-layer cost and edge(layer, previous, node)
// applies between layer-1 and layer. All transitions must be finite.
template<class Cost, class Initial, class Edge>
std::pair<Cost,std::vector<int>> ragged_layer_shortest_path(
    const std::vector<int>& widths, Initial initial, Edge edge) {
  if(widths.empty())return {Cost{}, {}};
  assert(widths.front()>0);
  std::vector<Cost> prev(widths.front()),cur;
  std::vector<std::vector<int>> parent(widths.size());
  for(int q=0;q<widths.front();++q)prev[q]=initial(q);
  for(int layer=1;layer<int(widths.size());++layer){
    assert(widths[layer]>0);
    cur.resize(widths[layer]);
    parent[layer].resize(widths[layer]);
    for(int q=0;q<widths[layer];++q){
      cur[q]=prev[0]+edge(layer,0,q);
      for(int p=1;p<int(prev.size());++p)
        if(Cost c=prev[p]+edge(layer,p,q);c<cur[q]){
          cur[q]=c;parent[layer][q]=p;
        }
    }
    cur.swap(prev);
  }
  std::vector<int> path(widths.size());
  int last=int(std::min_element(prev.begin(),prev.end())-prev.begin());
  const Cost val=prev[last];
  for(int k=int(widths.size())-1;k>=0;--k){
    path[k]=last;
    if(k)last=parent[k][last];
  }
  return {val,std::move(path)};
}
