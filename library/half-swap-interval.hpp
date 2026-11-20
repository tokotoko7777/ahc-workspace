#ifndef AHC_HALF_SWAP_INTERVAL_HPP
#define AHC_HALF_SWAP_INTERVAL_HPP
#include <algorithm>
#include <optional>
#include <utility>

// In usable half-open range [lo,hi), list all starts of length 2*|a-b|
// intervals whose equal-half swap exchanges slots a and b. Return inclusive
// endpoints of the contiguous start interval, or nullopt (including a==b).
inline std::optional<std::pair<int,int>> half_swap_interval_starts(int lo,int hi,int a,int b){
  if(a>b)std::swap(a,b);
  if(a<lo||b>=hi||a==b)return std::nullopt;
  const int delta=b-a;
  const int first=std::max(lo,a-delta+1),last=std::min(a,hi-2*delta);
  if(first>last)return std::nullopt;
  return std::pair{first,last};
}
#endif
