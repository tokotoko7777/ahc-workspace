#include "grouped-mask-packing.hpp"
#include <cassert>
#include <iostream>
#include <random>

using ahc::MaskPackingOption;
using Groups=std::vector<std::vector<MaskPackingOption>>;

std::int64_t brute_value(const Groups& g,std::size_t k=0,std::uint32_t used=0){
  if(k==g.size())return 0;
  std::int64_t best=brute_value(g,k+1,used);
  for(auto [bits,w]:g[k])if(!(used&bits))
    best=std::max(best,w+brute_value(g,k+1,used|bits));
  return best;
}
int main(){
  std::mt19937 rng(1143419);
  for(int it=0;it<5500;++it){
    int n=rng()%7;
    Groups g(rng()%7);
    for(auto& a:g)for(int j=0,end=rng()%5;j<end;++j)
      a.push_back({static_cast<std::uint32_t>(rng()%(1u<<n)),static_cast<int>(rng()%22)-5});
    const auto got=ahc::grouped_mask_packing(n,g);
    assert(got.value==brute_value(g)&&got.option.size()==g.size());
    std::uint32_t mask=0;std::int64_t total=0;
    for(std::size_t k=0;k<g.size();++k)if(int j=got.option[k];j>=0){
      assert(j<static_cast<int>(g[k].size()));
      auto [bits,w]=g[k][j];assert(!(mask&bits));mask|=bits;total+=w;
    }
    assert(mask==got.resources&&total==got.value);
    assert(ahc::grouped_mask_packing(n,g).option==got.option);
  }
  assert(ahc::grouped_mask_packing(16,Groups{{{0xffffu,2}}}).value==2);
  assert(ahc::grouped_mask_packing(0,Groups{{{0u,3}},{{0u,2}}}).value==5);
  try{ahc::grouped_mask_packing(2,Groups{{{4,0}}});assert(false);}
  catch(const std::invalid_argument&){}
  try{ahc::grouped_mask_packing(0,Groups{{{0,std::numeric_limits<std::int64_t>::max()}},{{0,1}}});assert(false);}
  catch(const std::overflow_error&){}
  std::cout << "5500 exhaustive differential fixtures and edge cases\n";
}
