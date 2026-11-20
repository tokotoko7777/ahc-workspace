#include <algorithm>
#include <cassert>
#include <random>
#include <vector>
#include "../library/nonbacktracking-path.hpp"

int main() {
  std::mt19937 rng(861500);
  for(int test=0;test<1600;++test){
    int n=2+rng()%9;
    std::vector<std::vector<int>> g(n);
    for(int a=0;a<n;++a)for(int b=a+1;b<n;++b)if(rng()%4==0){
      g[a].push_back(b);g[b].push_back(a);
    }
    int s=rng()%n,t=rng()%n,p=rng()%2||g[s].empty()?-1:g[s][rng()%g[s].size()];
    auto r=shortest_nonbacktracking_path(g,p,s,[&](int v){return v==t;});
    std::vector<std::vector<bool>> states(n+1,std::vector<bool>(n));
    states[p+1][s]=true;
    int exact=-1;
    for(int d=0;d<=n*n;++d){
      bool found=false;
      for(int a=0;a<=n;++a)found|=states[a][t];
      if(found){exact=d;break;}
      std::vector<std::vector<bool>> next(n+1,std::vector<bool>(n));
      for(int a=0;a<=n;++a)for(int b=0;b<n;++b)if(states[a][b])
        for(int c:g[b])if(c!=a-1)next[b+1][c]=true;
      next.swap(states);
    }
    assert(bool(r)==(exact>=0));
    if(r){
      assert(int(r->size())==exact);
      for(int u:*r){
        assert(u!=p&&std::find(g[s].begin(),g[s].end(),u)!=g[s].end());
        p=s;s=u;
      }
      assert(s==t);
    }
  }
}
