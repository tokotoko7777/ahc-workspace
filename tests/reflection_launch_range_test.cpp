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
  for(int n=1;n<=13;++n)for(int a=0;a<n;++a)for(int b=0;b<n;++b)
      for(int c=0;c<n;++c)for(int d=0;d<n;++d){
    int best=INT_MAX;
    for(int x=0;x<n;++x){
      int y=2*c-x,z=2*d-y;
      if(0<=y&&y<n&&0<=z&&z<n)best=std::min(best,std::abs(a-x)+std::abs(b-z));
    }
    const auto got=double_reflection_launch_range(n,a,b,c,d);
    assert(bool(got)==(best<INT_MAX));
    for(int x=0;x<n;++x){
      int y=2*c-x,z=2*d-y;
      const bool opt=0<=y&&y<n&&0<=z&&z<n&&std::abs(a-x)+std::abs(b-z)==best;
      assert(opt==(got&&got->first<=x&&x<=got->second));
    }
  }
}
