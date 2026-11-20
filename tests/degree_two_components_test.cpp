#include <algorithm>
#include <array>
#include <cassert>
#include <numeric>
#include <queue>
#include <random>
#include <tuple>
#include <vector>
#include "../library/degree-two-components.hpp"

int main() {
  std::mt19937 rng(100712);
  for (int test = 0; test < 500; ++test) {
    const int n = 1 + rng()%100;
    DegreeTwoComponents scan(n);
    for (int iteration=0; iteration<8; ++iteration) {
      std::vector<int> order(n);
      std::iota(order.begin(), order.end(), 0);
      std::shuffle(order.begin(), order.end(), rng);
      std::vector<bool> active(n);
      std::vector<std::array<int,2>> adj(n, {-1,-1});
      auto edge = [&](int a,int b) {
        auto add = [&](int v, int w) { adj[v][adj[v][0] >=0] = w; };
        add(a,b);
        add(b,a);
      };
      for (int i=0;i<n;) {
        const int k=std::min(n-i, 1+int(rng()%9));
        if (rng()%5!=0) {
          for (int j=0;j<k;++j) active[order[i+j]]=true;
          for (int j=1;j<k;++j) edge(order[i+j-1],order[i+j]);
          if (rng()%2) edge(order[i],order[i+k-1]);
        }
        i+=k;
      }
      using Entry=std::tuple<int,int,int>;
      std::vector<Entry> found, expected;
      scan.scan([&](int v){return active[v];},
                [&](int v){return adj[v];},
                [&](int len,int a,int b){found.emplace_back(len,std::min(a,b),std::max(a,b));});
      std::vector<bool> seen(n);
      for(int v=0;v<n;++v) if(active[v]&&!seen[v]){
        std::queue<int> q;
        q.push(v);seen[v]=true;
        int count=0;
        std::vector<int> ends;
        while(!q.empty()){
          int u=q.front();q.pop();++count;
          for(int w:adj[u]){
            if(w<0)ends.push_back(u);
            else if(!seen[w])q.push(w),seen[w]=true;
          }
        }
        std::sort(ends.begin(),ends.end());
        assert(ends.empty()||ends.size()==2);
        expected.emplace_back(count,ends.empty()?-1:ends[0],ends.empty()?-1:ends[1]);
      }
      std::sort(found.begin(),found.end());
      std::sort(expected.begin(),expected.end());
      assert(found==expected);
    }
  }
}
