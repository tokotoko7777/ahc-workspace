#include "../library/exact-step-frontier.hpp"
#include <iostream>

int main(){
    const std::array<std::array<int,2>,4> road{{{-1,1},{0,2},{1,3},{2,-1}}};
    // Starting at vertex 0: walk exactly two steps, rather than at most two.
    const auto end=ahc::exact_step_frontier(road,std::bitset<4>{},std::bitset<4>{1},1u<<2);
    std::cout<<end<<'\n'; // 0101: after exactly two steps, only 0 and 2.
}
