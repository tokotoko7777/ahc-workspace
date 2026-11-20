#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <random>
#include <vector>
#include "../library/incremental-manhattan-distance.hpp"

int main(){
  std::mt19937 rng(70371);
  for(int test=0;test<120;++test){
    int h=1+rng()%20,w=1+rng()%22;
    IncrementalManhattanDistance d(h,w);
    std::vector<int> source;
    for(int i=0;i<50;++i){
      const int p=rng()%(h*w);
      assert(d.add(p)==(std::find(source.begin(),source.end(),p)==source.end()));
      source.push_back(p);
      for(int q=0;q<h*w;++q){
        int best=h+w;
        for(int s:source)best=std::min(best,std::abs(s/w-q/w)+std::abs(s%w-q%w));
        assert(d.distance(q)==best);
      }
    }
  }
}
