#include <cassert>
#include <climits>
#include <cstdlib>
#include "../library/reflection-launch-range.hpp"

int main(){
  for(int n=1;n<=19;++n)for(int a=0;a<n;++a)for(int b=0;b<n;++b)for(int c=0;c<n;++c){
    const auto [lo,hi]=reflection_launch_range(n,a,b,c);
    int best=INT_MAX;
    for(int x=0;x<n;++x)
      if(int y=2*c-x;0<=y&&y<n)best=std::min(best,std::abs(a-x)+std::abs(b-y));
    for(int x=0;x<n;++x){
      const int y=2*c-x;
      const bool good=0<=y&&y<n&&std::abs(a-x)+std::abs(b-y)==best;
      assert(good==(lo<=x&&x<=hi));
    }
  }
}
