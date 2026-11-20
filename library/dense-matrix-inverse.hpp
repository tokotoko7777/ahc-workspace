#pragma once
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

// Gauss-Jordan inverse with partial pivoting and an absolute pivot guard.
// Use a scale-aware pivot_floor for matrices not on a natural unit scale.
template<class Real,std::size_t N>
std::optional<std::array<std::array<Real,N>,N>> dense_matrix_inverse(
    std::array<std::array<Real,N>,N> a,Real pivot_floor) {
  static_assert(std::is_floating_point_v<Real>);
  std::array<std::array<Real,N>,N> b{};
  for(std::size_t i=0;i<N;++i)b[i][i]=1;
  for(std::size_t k=0;k<N;++k){
    std::size_t pivot=k;
    for(std::size_t i=k+1;i<N;++i)
      if(std::abs(a[i][k])>std::abs(a[pivot][k]))pivot=i;
    if(!(std::abs(a[pivot][k])>pivot_floor))return std::nullopt;
    std::swap(a[k],a[pivot]);std::swap(b[k],b[pivot]);
    const Real inv=1/a[k][k];
    for(std::size_t j=k;j<N;++j)a[k][j]*=inv;
    for(std::size_t j=0;j<N;++j)b[k][j]*=inv;
    for(std::size_t i=0;i<N;++i)if(i!=k){
      const Real amount=a[i][k];
      if(amount==0)continue;
      for(std::size_t j=k;j<N;++j)a[i][j]-=amount*a[k][j];
      for(std::size_t j=0;j<N;++j)b[i][j]-=amount*b[k][j];
    }
  }
  return b;
}
