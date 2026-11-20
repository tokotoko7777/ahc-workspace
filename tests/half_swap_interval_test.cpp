#include <cassert>
#include <vector>
#include "../library/half-swap-interval.hpp"

int main(){
  for(int lo=-4;lo<=4;++lo)for(int hi=lo+1;hi<lo+21;++hi)
    for(int a=lo-1;a<=hi;++a)for(int b=lo-1;b<=hi;++b){
      const int d=a>b?a-b:b-a;
      std::vector<int> exact;
      if(d>0)for(int start=lo;start+2*d<=hi;++start){
        if(a<start||a>=start+2*d)continue;
        int mapped=(a-start<d)?a+d:a-d;
        if(mapped==b)exact.push_back(start);
      }
      auto result=half_swap_interval_starts(lo,hi,a,b);
      assert(bool(result)==!exact.empty());
      if(result){
        assert((std::pair{exact.front(),exact.back()}==*result));
        assert(int(exact.size())==result->second-result->first+1);
      }
    }
}
