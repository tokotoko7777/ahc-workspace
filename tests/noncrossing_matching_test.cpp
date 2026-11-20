#include <algorithm>
#include <cassert>
#include <random>
#include <vector>
#include "../library/noncrossing-matching.hpp"

int main(){
  std::mt19937 rng(64225);
  struct Edge{int left,right,weight;};
  for(int test=0;test<1800;++test){
    int n=rng()%13;
    std::vector<Edge> e(n);
    for(auto& v:e)v={int(rng()%7),int(rng()%8),int(rng()%50)-10};
    auto p=maximum_weight_noncrossing_matching(e);
    int best=0,actual=0;
    for(unsigned mask=0;mask<(1u<<n);++mask){
      bool legal=true;int value=0;
      for(int i=0;i<n;++i)if(mask>>i&1){
        value+=e[i].weight;
        for(int j=0;j<i;++j)if(mask>>j&1){
          legal&=(e[i].left<e[j].left&&e[i].right<e[j].right)||
                  (e[i].left>e[j].left&&e[i].right>e[j].right);
        }
      }
      if(legal)best=std::max(best,value);
    }
    for(int i=0;i<int(p.size());++i){
      actual+=e[p[i]].weight;
      if(i){assert(e[p[i-1]].left<e[p[i]].left&&e[p[i-1]].right<e[p[i]].right);}
    }
    assert(actual==best);
  }
}
