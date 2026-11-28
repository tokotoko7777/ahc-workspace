#include "../library/grid-row-flood.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
template <int H, int W, class U>
void verify() {
  std::mt19937_64 rng(987531);
  for (int rep = 0; rep < 2500; ++rep) {
    std::array<U, H> open{}, expected{};
    for (auto& row : open) {
      // Include masks with meaningless padding bits and varied openness.
      row = rep%7==0 ? ~U(0) : rep%5==0 ? U(0) : rep%3==0 ? rng() : (rng() | rng());
    }
    int r = rng()%H, c = rng()%W;
    std::vector<int> queue;
    auto add = [&](int y, int x) {
      if (y < 0 || y >= H || x < 0 || x >= W) return;
      const U b = U(U{1} << x);
      if ((open[y] & b) && !(expected[y] & b)) { expected[y] |= b; queue.push_back(y*W+x); }
    };
    add(r,c);
    for (std::size_t i=0;i<queue.size();++i) {
      const int p=queue[i], y=p/W, x=p%W;
      add(y-1,x);add(y+1,x);add(y,x-1);add(y,x+1);
    }
    assert((ahc::row_flood<H,W>(open,r,c)==expected));
    assert((ahc::row_flood<H,W>(open,H,c)==std::array<U,H>{}));
    assert((ahc::row_flood<H,W>(open,r,W)==std::array<U,H>{}));
  }
}
int main(){
 verify<1,1,std::uint32_t>();verify<1,32,std::uint32_t>();verify<30,30,std::uint32_t>();
 verify<7,64,std::uint64_t>();verify<18,11,std::uint32_t>();verify<17,16,std::uint16_t>();
 std::cout<<"15000 comparisons passed\n";
}
