#include "../library/overlapping-interval-max-transform.hpp"
#include <cassert>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>

int main(){
  std::mt19937 rng(39139);
  for(int trial=0;trial<2400;++trial){
    int n=2+rng()%13;
    std::vector<int> cut(n);
    for(int i=1;i<n;++i)cut[i]=cut[i-1]+1+rng()%53;
    std::vector<long long> value(n*n);
    std::vector<int> source(n*n,-1);
    for(int i=0;i<n;++i)for(int j=i+1;j<n;++j)if(rng()%4){
      value[i*n+j]=int(rng()%1901)-950;
      source[i*n+j]=i*n+j+n*n;
    }
    auto original=value;
    auto available=source;
    long long p=rng()%17;
    overlapping_interval_max_transform(cut,p,value,source);
    for(int l=0;l<n;++l)for(int h=l+1;h<n;++h){
      auto calc=[&](int a,int b){return original[a*n+b]-p*(std::abs(cut[l]-cut[a])+std::abs(cut[h]-cut[b]));};
      long long optimum=std::numeric_limits<long long>::min();
      for(int a=0;a<n;++a)for(int b=a+1;b<n;++b)
        if(available[a*n+b]>=0&&std::max(a,l)<std::min(b,h))
          optimum=std::max(optimum,calc(a,b));
      int s=source[l*n+h];
      if(optimum==std::numeric_limits<long long>::min())assert(s==-1);
      else{
        assert(s>=n*n);
        int k=s-n*n,a=k/n,b=k%n;
        assert(available[k]>=0&&std::max(a,l)<std::min(b,h));
        assert(calc(a,b)==optimum&&value[l*n+h]==optimum);
      }
    }
  }
}
