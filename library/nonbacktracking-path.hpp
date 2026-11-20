#ifndef AHC_NONBACKTRACKING_PATH_HPP
#define AHC_NONBACKTRACKING_PATH_HPP
#include <algorithm>
#include <cassert>
#include <optional>
#include <vector>

// Shortest path on a small unweighted graph when immediately reversing an
// edge is forbidden. previous==-1 means there is no previous vertex.
// Accept(v) is tested for start and after each move. Returns only destinations
// of movements. A valid empty route (start already accepted) differs from
// nullopt (no route). The BFS has at most one state per directed edge plus
// the start state; this simple dense-index version uses O(V^2) memory.
template<class Graph,class Accept>
std::optional<std::vector<int>> shortest_nonbacktracking_path(
    const Graph& graph,int previous,int start,Accept accept) {
  const int n=int(graph.size());
  assert(0<=start&&start<n&&-1<=previous&&previous<n);
  const int origin=previous<0?n*n:previous*n+start;
  std::vector<int> parent(n*n+1,-1),queue;
  queue.push_back(origin);
  parent[origin]=origin;
  for(std::size_t head=0;head<queue.size();++head){
    const int e=queue[head],cur=e==n*n?start:e%n,prev=e==n*n?-1:e/n;
    if(accept(cur)){
      std::vector<int> result;
      for(int j=e;j!=origin;j=parent[j])result.push_back(j%n);
      std::reverse(result.begin(),result.end());
      return result;
    }
    for(int u:graph[cur])if(u!=prev){
      assert(0<=u&&u<n);
      const int next=cur*n+u;
      if(parent[next]<0){
        parent[next]=e;
        queue.push_back(next);
      }
    }
  }
  return std::nullopt;
}
#endif
