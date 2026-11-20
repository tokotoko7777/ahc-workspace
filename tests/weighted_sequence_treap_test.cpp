#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <vector>
#include "../library/weighted-sequence-treap.hpp"

int main(){
  std::mt19937 rng(622954);
  for(int trial=0;trial<160;++trial){
    int n=2+rng()%150;
    std::vector<int> v(n),w(n);
    std::iota(v.begin(),v.end(),0);std::shuffle(v.begin(),v.end(),rng);
    for(auto& x:w)x=int(rng()%20001)-10000;
    WeightedSequenceTreap s(v,w);
    auto score=[&](const auto& p){
      long long z=0;
      for(int i=0;i<n;++i)z+=i*1LL*w[p[i]];
      return z;
    };
    for(int it=0;it<180;++it){
      int l=rng()%(n-1),r=l+2+rng()%(n-l-1);
      auto p=v;
      std::reverse(p.begin()+l,p.begin()+r);
      bool chosen=rng()%2;
      assert(s.try_reverse(l,r,[&](auto difference){
         assert(difference==score(p)-score(v));
         return chosen;
      })==chosen);
      if(chosen)v.swap(p);
      assert(s.indexed_sum()==score(v));
      const int j=rng()%n;
      assert(s.rank(v[j])==j&&s.at(j)==v[j]);
      if(it%17==0)assert(s.order()==v);
    }
    assert(s.order()==v);
  }
}
