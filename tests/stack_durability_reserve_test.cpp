#include "../library/stack-durability-reserve.hpp"
#include <cassert>
#include <cstdlib>
#include <random>
#include <vector>

int main(){
  std::mt19937 rng(490490);
  constexpr int z=16;
  for(int t=0;t<2200;++t){
    StackDurabilityReserve<z> s;
    std::vector<long long> weights,remain;
    int y=0,x=0;
    for(int it=0;it<38;++it){
      int a=rng()%20,b=rng()%20;
      int len=std::abs(y-a)+std::abs(x-b);
      long long w=1+rng()%1000,d=1+rng()%70000;
      auto q=remain;
      for(int i=0;i<int(q.size());++i)
        for(int j=i+1;j<int(weights.size());++j)q[i]-=len*weights[j];
      q.push_back(d);
      auto next=weights;next.push_back(w);
      auto final=q;
      for(int i=0;i<int(q.size());++i)
        for(int j=i+1;j<int(next.size());++j)final[i]-=(a+b)*next[j];
      bool good=int(q.size())<=z;
      for(long long f:final)good&=f>0;
      auto before=s;
      assert(s.append(w,d,len,a+b)==good);
      if(good){
        assert(s.size==int(final.size()));
        for(int i=0;i<s.size;++i)assert(s.layer[i].margin==final[i]);
        y=a;x=b;remain=q;weights=next;
      }else{
        assert(s.size==before.size&&s.home==before.home);
        for(int i=0;i<s.size;++i)
          assert(s.layer[i].margin==before.layer[i].margin&&s.layer[i].above==before.layer[i].above);
      }
    }
  }
}
