#include "../library/dense-matrix-inverse.hpp"
#include <cassert>
#include <random>

int main(){
  auto singular=dense_matrix_inverse(std::array{
    std::array{1.,2.},std::array{2.,4.}},1e-13);
  assert(!singular);
  auto asymmetric=dense_matrix_inverse(std::array{
    std::array{0.,3.},std::array{2.,4.}},1e-13);
  assert(asymmetric && std::abs((*asymmetric)[0][1]-.5)<1e-12);
  std::mt19937 rng(44046);
  for(int t=0;t<900;++t){
    constexpr int n=6;
    std::array<std::array<double,n>,n> a;
    for(int i=0;i<n;++i){
      for(int j=0;j<n;++j)a[i][j]=double(int(rng()%100)-50)/11;
      a[i][i]+=40;
    }
    if(t%2)std::swap(a[0],a[3]);
    auto ans=dense_matrix_inverse(a,1e-13);
    assert(ans);
    for(int i=0;i<n;++i)for(int j=0;j<n;++j){
      double v=0;
      for(int k=0;k<n;++k)v+=a[i][k]*(*ans)[k][j];
      assert(std::abs(v-double(i==j))<1e-11);
    }
  }
}
