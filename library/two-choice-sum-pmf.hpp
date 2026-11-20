#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

struct TwoChoiceSumPmf {
  int offset=0;
  std::vector<std::uint64_t> count{1};
  std::uint64_t outcomes=1;
};

// Each integer pair contributes one of its two entries independently,
// with probability 1/2. Returns exact counts and a power-of-two divisor.
// Caller supplies a manageable value range and <= 62 pairs.
inline TwoChoiceSumPmf two_choice_sum_pmf(
    const std::vector<std::pair<int,int>>& values) {
  assert(values.size()<=62);
  TwoChoiceSumPmf pmf;
  for(auto [a,b]:values){
    if(a>b)std::swap(a,b);
    pmf.offset+=a;
    const std::size_t d=static_cast<std::size_t>(static_cast<long long>(b)-a);
    const std::size_t previous=pmf.count.size();
    pmf.count.resize(previous+d);
    // Reverse update leaves each old count intact until it is read.
    for(std::size_t i=previous;i>0;--i)pmf.count[i-1+d]+=pmf.count[i-1];
    pmf.outcomes*=2;
  }
  return pmf;
}
