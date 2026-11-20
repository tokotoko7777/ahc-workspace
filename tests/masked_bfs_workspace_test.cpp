#include <cassert>
#include <queue>
#include <random>
#include <vector>
#include "../library/masked-bfs-workspace.hpp"

int main() {
  std::mt19937 rng(546368);
  for(int graph_case=0;graph_case<300;++graph_case){
    int n=1+rng()%70;
    std::vector<std::vector<int>> graph(n);
    for(int v=0;v<n;++v)for(int u=0;u<n;++u)if(rng()%8==0)graph[v].push_back(u);
    MaskedBfsWorkspace live(n);
    for(int rep=0;rep<10;++rep){
      std::vector<char> allow(n);
      for(auto& x:allow)x=(rng()%3)>0;
      int from=rng()%n;
      live.run(graph,from,[&](int v){return allow[v];});
      std::vector<int> d(n,-1),p(n,-1);
      std::queue<int> q;
      if(allow[from]){q.push(from);d[from]=0;p[from]=from;}
      while(!q.empty()){
        int v=q.front();q.pop();
        for(int u:graph[v])if(allow[u]&&d[u]<0){d[u]=d[v]+1;p[u]=v;q.push(u);}
      }
      for(int v=0;v<n;++v){assert(live.distance(v)==d[v]);assert(live.parent(v)==p[v]);}
    }
  }
}
