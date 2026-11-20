#include <cassert>
#include <cmath>
#include "../library/rounded-clipped-normal.hpp"

int main() {
  using std::abs;
  using std::exp;
  auto mass=[](int v,double m,double s,int lo=0,int hi=1000){
    return exp(rounded_clipped_normal_logmass(v,m,s,lo,hi));
  };
  for(int m:{0,1,300,500,999,1000})for(int s:{1,4,17,90,500,900}){
    double sum=0;
    for(int v=0;v<=1000;++v){
      double p=mass(v,m,s);
      assert(p>0 && p<=1);
      assert(abs(p-mass(1000-v,1000-m,s))<1e-14);
      sum+=p;
    }
    assert(abs(sum-1)<1e-12);
  }
  assert(mass(0,0,900)>0.5);
  assert(mass(1000,1000,900)>0.5);
  assert(mass(0,20,3,0,0)==1);
  // Uniform shift of rounded buckets and endpoints does not change probability.
  for(int v=0;v<=20;++v)
    assert(abs(mass(v,12.3,4,0,20)-mass(v-3,9.3,4,-3,17))<1e-14);
}
