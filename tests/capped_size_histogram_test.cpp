#include <algorithm>
#include <cassert>
#include <random>
#include <vector>
#include "../library/capped-size-histogram.hpp"

int main() {
  std::mt19937 rng(120448);
  for(int t=0;t<250;++t){
    std::vector<int> cap(1+rng()%22);
    for(int i=1;i<int(cap.size());++i)cap[i]=rng()%10;
    CappedSizeHistogram hist(cap);
    std::vector<int> sizes(1+rng()%75);
    for(int j=0;j<400;++j){
      int p=rng()%sizes.size(), old=sizes[p], now=rng()%32;
      hist.replace_size(old,now);
      sizes[p]=now;
      if(rng()%4==0){
        hist.replace_size(now,old);
        sizes[p]=old;
      }
      int expected=0;
      for(int i=0;i<int(cap.size());++i){
        auto count=std::count(sizes.begin(),sizes.end(),i);
        if(cap[i]>0)assert(hist.frequency(i)==count);
        expected+=std::min(cap[i],int(count));
      }
      assert(hist.value()==expected);
    }
  }
}
