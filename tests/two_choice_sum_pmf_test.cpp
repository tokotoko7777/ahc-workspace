#include "../library/two-choice-sum-pmf.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>
#include <numeric>
#include <random>

int main(){
  std::mt19937 rng(35235);
  for(int t=0;t<900;++t){
    std::vector<std::pair<int,int>> items(rng()%11);
    for(auto& [a,b]:items){a=int(rng()%41)-20;b=int(rng()%41)-20;}
    auto result=two_choice_sum_pmf(items);
    std::map<int,std::uint64_t> exact;
    for(unsigned mask=0;mask<(1u<<items.size());++mask){
      int x=0;
      for(std::size_t j=0;j<items.size();++j)x+=mask&(1u<<j)?items[j].first:items[j].second;
      ++exact[x];
    }
    assert((1ull<<items.size())==result.outcomes);
    assert(exact.begin()->first==result.offset);
    assert(exact.rbegin()->first==result.offset+int(result.count.size())-1);
    for(std::size_t i=0;i<result.count.size();++i)
      assert(result.count[i]==exact[result.offset+int(i)]);
  }
  // Tail-sum identity used by AHC035 for independent child maxima.
  for(int t=0;t<400;++t){
    std::vector<std::pair<int,int>> a(3),b(3);
    for(auto& [x,y]:a){x=rng()%15;y=rng()%15;}
    for(auto& [x,y]:b){x=rng()%15;y=rng()%15;}
    auto A=two_choice_sum_pmf(a),B=two_choice_sum_pmf(b);
    int lo=std::min(A.offset,B.offset),hi=std::max(A.offset+int(A.count.size())-1,B.offset+int(B.count.size())-1);
    double moment=hi;int ca=0,cb=0,enumeration=0;
    for(int q=lo;q<hi;++q){
      if(q>=A.offset&&q<A.offset+int(A.count.size()))ca+=A.count[q-A.offset];
      if(q>=B.offset&&q<B.offset+int(B.count.size()))cb+=B.count[q-B.offset];
      if(ca&&cb)moment-=std::exp(std::log(ca/8.)+std::log(cb/8.));
    }
    for(std::size_t i=0;i<A.count.size();++i)for(std::size_t j=0;j<B.count.size();++j)
      enumeration+=A.count[i]*B.count[j]*std::max(A.offset+int(i),B.offset+int(j));
    assert(std::abs(moment-enumeration/64.)<1e-12);
  }
}
