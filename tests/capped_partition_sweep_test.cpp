#include <algorithm>
#include <cassert>
#include <set>
#include <random>
#include "../library/capped-partition-sweep.hpp"

int main() {
  std::mt19937 rng(120561);
  for (int t=0;t<800;++t) {
    std::vector<int> cap(15), size(1+rng()%10);
    for(int i=1;i<int(cap.size());++i)cap[i]=rng()%10;
    std::vector<std::pair<int,int>> events;
    std::set<int> keys;
    for(int k=0;k<40;++k){
      int p=int(rng()%17)-8, g=rng()%size.size();
      keys.insert(p);
      ++size[g];
      events.emplace_back(p,g);
    }
    std::sort(events.begin(),events.end());
    const auto reference=[&](int th){
      std::vector<int> low(size.size()),high(size.size()),freq(cap.size());
      for(auto [p,g]:events)++(p<=th?low:high)[g];
      for(auto side:{low,high})for(int n:side)if(n<int(cap.size()))++freq[n];
      int v=0;
      for(int i=1;i<int(cap.size());++i)v+=std::min(freq[i],cap[i]);
      return v;
    };
    auto actual=best_capped_partition_split(events,size,cap);
    assert(actual.initial_value==reference(-9));
    int best=actual.initial_value;
    std::optional<int> threshold;
    for(int k:keys){
      int v=reference(k);
      if(v>best)best=v,threshold=k;
    }
    assert(actual.threshold==threshold && actual.value==best);
  }
}
