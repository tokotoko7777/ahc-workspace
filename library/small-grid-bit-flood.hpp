#pragma once
#include <bit>
#include <cassert>
#include <cstdint>

// GCC/Clang の unsigned 128-bit で小さいグリッドの4近傍を表現。
// 各行の末尾に番兵1bit。H*(W+1)<=128。実際のセルだけを 1 にして使う。
template<int H,int W>
struct SmallGridBitFlood {
  static_assert(H>0 && W>0 && H*(W+1)<=128);
  using Mask=__uint128_t;
  static constexpr int stride=W+1;
  static constexpr Mask cell(int r,int c){return Mask(1)<<(r*stride+c);}
  static constexpr Mask valid=[] {
    Mask m=0;
    for(int r=0;r<H;++r)for(int c=0;c<W;++c)m|=cell(r,c);
    return m;
  }();
  static constexpr Mask adjacent(Mask a) {
    return ((a<<1)|(a>>1)|(a<<stride)|(a>>stride))&valid;
  }
  static int count(Mask m) {
    return std::popcount(std::uint64_t(m))+std::popcount(std::uint64_t(m>>64));
  }
  template<class Visit>
  static void components(Mask remaining,Visit visit){
    assert(!(remaining&~valid));
    while(remaining){
      Mask component=remaining&(-remaining),frontier=component;
      remaining^=component;
      while(frontier){
        frontier=adjacent(frontier)&remaining;
        remaining^=frontier;
        component|=frontier;
      }
      visit(component);
    }
  }
};
