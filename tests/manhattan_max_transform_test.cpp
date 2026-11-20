#include "../library/manhattan-max-transform.hpp"
#include <cassert>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>

int main(){
  std::mt19937 rng(39039);
  for(int t=0;t<2000;++t){
    int n=1+rng()%10,m=1+rng()%10;
    std::vector<int> x(n),y(m);
    for(int i=1;i<n;++i)x[i]=x[i-1]+1+rng()%21;
    for(int i=1;i<m;++i)y[i]=y[i-1]+1+rng()%21;
    std::vector<long long> init(n*m);
    std::vector<int> src(n*m,-1);
    for(int k=0;k<n*m;++k)if(rng()%5){
      init[k]=static_cast<long long>(rng()%401)-200;
      src[k]=k;
    }
    auto val=init;
    auto old=src;
    const long long p=rng()%16;
    manhattan_max_transform(x,y,p,val,src);
    for(int k=0;k<n*m;++k){
      auto score=[&](int i){
        return init[i]-p*(std::abs(x[k/m]-x[i/m])+std::abs(y[k%m]-y[i%m]));
      };
      long long best=std::numeric_limits<long long>::min();
      for(int i=0;i<n*m;++i)if(old[i]>=0)best=std::max(best,score(i));
      if(best==std::numeric_limits<long long>::min())assert(src[k]==-1);
      else assert(src[k]>=0&&old[src[k]]>=0&&score(src[k])==best&&val[k]==best);
    }
  }
}
