#pragma once
#include <algorithm>
#include <cassert>
#include <optional>
#include <vector>

template<class Cost>
struct BoundedLayerPath {
  Cost cost;
  std::vector<int> states;
};

// h層×w状態。隣層間の状態差はmax_jump以下。
// node(y,x), edge(y,prev,x) は optional<Cost>; nullopt は禁止。
template<class Cost,class Node,class Edge>
std::optional<BoundedLayerPath<Cost>> bounded_layer_shortest_path(
    int h,int w,int max_jump,Node node,Edge edge) {
  assert(h>0&&w>0&&max_jump>=0);
  std::vector<int> parent(h*w,-1);
  std::vector<std::optional<Cost>> before(w),next(w);
  for(int x=0;x<w;++x)before[x]=node(0,x);
  for(int y=1;y<h;++y){
    std::fill(next.begin(),next.end(),std::nullopt);
    for(int x=0;x<w;++x)if(auto c=node(y,x)){
      for(int z=std::max(0,x-max_jump);z<=std::min(w-1,x+max_jump);++z)
        if(before[z])if(auto d=edge(y,z,x)){
          Cost total=*before[z]+*c+*d;
          if(!next[x]||total<*next[x])next[x]=total,parent[y*w+x]=z;
        }
    }
    before.swap(next);
  }
  int end=-1;
  for(int x=0;x<w;++x)if(before[x]&&(end<0||*before[x]<*before[end]))end=x;
  if(end<0)return std::nullopt;
  BoundedLayerPath<Cost> answer{*before[end],std::vector<int>(h)};
  for(int y=h-1;y>=0;--y){
    answer.states[y]=end;
    if(y)end=parent[y*w+end];
  }
  return answer;
}
