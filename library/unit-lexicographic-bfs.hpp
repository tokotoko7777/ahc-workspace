#ifndef AHC_UNIT_LEXICOGRAPHIC_BFS_HPP
#define AHC_UNIT_LEXICOGRAPHIC_BFS_HPP
#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>

struct UnitLexicographicBfs {
  struct Edge {int to,secondary;};
  std::vector<int> length,secondary,parent,edge_index;
  explicit UnitLexicographicBfs(int n)
      :length(n),secondary(n),parent(n),edge_index(n){ queue.reserve(n); }

  // Primary cost is one per edge. Minimize a nonnegative integer secondary
  // cost among equal primary distances. O(V+E), preserving predecessor/edge.
  // Secondary cost sums must fit int. Source gets length=secondary=0.
  template<class Graph>
  void run(const Graph& graph,int source) {
    assert(0<=source&&source<int(length.size()));
    std::fill(length.begin(),length.end(),-1);
    std::fill(secondary.begin(),secondary.end(),std::numeric_limits<int>::max());
    std::fill(parent.begin(),parent.end(),-1);
    std::fill(edge_index.begin(),edge_index.end(),-1);
    queue.clear();queue.push_back(source);
    length[source]=secondary[source]=0;
    for(std::size_t i=0;i<queue.size();++i){
      int v=queue[i],idx=0;
      for(auto e:graph[v]){
        int u=e.to,z=secondary[v]+e.secondary;
        if(length[u]<0){length[u]=length[v]+1;queue.push_back(u);}
        if(length[u]==length[v]+1&&z<secondary[u]){
          secondary[u]=z;parent[u]=v;edge_index[u]=idx;
        }
        ++idx;
      }
    }
  }
  std::vector<int> edges_to(int target)const{
    if(length[target]<0)return {};
    std::vector<int> result;
    for(int v=target;parent[v]>=0;v=parent[v])result.push_back(edge_index[v]);
    std::reverse(result.begin(),result.end());return result;
  }

private:
  std::vector<int> queue;
};
#endif
