#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC optimize("O3")
#endif
#include "../problems/ahc004/anchor-congruence.hpp"
#include <array>
#include <cassert>
#include <random>
int main(){
  std::array<uint64_t,20> a{};
  a[0]=3;
  std::array<int,20> ordinary,preferred;
  assert(ahc004::match_columns(a,&ordinary)==1 && ordinary[0]==0);
  assert(ahc004::match_columns<true>(a,&preferred,2)==1 && preferred[0]==1);
  // Native preference cannot block an augmenting path needed for max cardinality.
  a[1]=2;
  assert(ahc004::match_columns<true>(a,&preferred,2)==2);
  assert((preferred[0]==0 && preferred[1]==1));
  std::mt19937_64 rng(112586);
  for(int t=0;t<300;++t){
    for(auto& mask:a) mask=rng()&rng()&rng()&rng()&rng();
    assert(ahc004::match_columns(a,&ordinary)==ahc004::match_columns<true>(a,&preferred,rng()));
    uint64_t seen=0;
    for(int i=0;i<20;++i) if(preferred[i]>=0) {
      uint64_t bit=uint64_t(1)<<preferred[i];
      assert((a[i]&bit) && !(seen&bit));
      seen|=bit;
    }
  }
  for(int t=0;t<150;++t){
    a.fill(0);
    for(int i=0;i<5;++i)a[i]=rng()&127;
    uint64_t native=rng()&127;
    int got=ahc004::match_columns<true>(a,&preferred,native), fav=0;
    for(int id:preferred)if(id>=0)fav+=(native>>id)&1;
    bool can[128]{true};
    for(int i=0;i<5;++i)for(int mask=127;mask>=0;--mask)if(can[mask])
      for(int j=0;j<7;++j)if(!(mask>>j&1)&&(a[i]>>j&1))can[mask|(1<<j)]=true;
    std::pair<int,int> opt{};
    for(unsigned mask=0;mask<128;++mask)if(can[mask])
      opt=std::max(opt,{std::popcount(mask),std::popcount(mask&native)});
    assert((std::pair{got,fav}==opt));
  }
}
