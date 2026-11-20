#include <array>
#include <cassert>
#include <functional>
#include <queue>
#include <random>
#include <tuple>
#include <vector>
#include "../library/unit-lexicographic-bfs.hpp"

int main(){
  std::mt19937 rng(661708);
  for(int it=0;it<1800;++it){
    int n=1+rng()%32,start=rng()%n;
    std::vector<std::vector<UnitLexicographicBfs::Edge>> g(n);
    for(int i=0;i<n;++i)for(int j=0;j<n;++j)if(rng()%5==0)
      g[i].push_back({j,int(rng()%66)});
    UnitLexicographicBfs b(n);b.run(g,start);
    std::vector<std::pair<int,int>> d(n,{123456,123456});d[start]={0,0};
    using N=std::tuple<int,int,int>;
    std::priority_queue<N,std::vector<N>,std::greater<N>> q;q.emplace(0,0,start);
    while(!q.empty()){
      auto [a,c,v]=q.top();q.pop();
      if(d[v]!=std::pair{a,c})continue;
      for(auto e:g[v]){
        std::pair z{a+1,c+e.secondary};
        if(z<d[e.to]){d[e.to]=z;q.emplace(z.first,z.second,e.to);}
      }
    }
    for(int i=0;i<n;++i){
      if(d[i].first==123456){assert(b.length[i]<0);continue;}
      assert((d[i]==std::pair{b.length[i],b.secondary[i]}));
      const auto e=b.edges_to(i);
      assert(int(e.size())==b.length[i]);
      int v=start,cost=0;
      for(int j:e){cost+=g[v][j].secondary;v=g[v][j].to;}
      assert(v==i&&cost==b.secondary[i]);
    }
  }
}
